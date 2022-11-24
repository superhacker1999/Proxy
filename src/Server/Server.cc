#include "Server.h"

/**
 * Server constructor
 * Creates an fd for listening socket,
 * sets this socket to listen with queue of 10 clients
 * @param server_port The port that server will use as listening to connect new
 * users
 * @param db_ip IP of Database
 * @param db_port port of Database
 */
tcp::Server::Server(int server_port, char* db_ip, int db_port)
    : m_fds_counter_(1),
      m_compress_arr_(false),
      DB_IP(db_ip),
      DB_PORT(db_port),
      m_logger_(new Logger("logs/log.txt")) {
  m_listening_server_fd_ = tcp::NetHandler::Socket(AF_INET, SOCK_STREAM, 0);
  tcp::NetHandler::MakeSocketReuseable(m_listening_server_fd_);
  tcp::NetHandler::SetNonBlockingSocket(m_listening_server_fd_);
  auto addr = tcp::NetHandler::InitAddr(server_port);
  tcp::NetHandler::Bind(m_listening_server_fd_, (sockaddr*)&addr, sizeof(addr));
  tcp::NetHandler::Listen(m_listening_server_fd_, 10);
  m_fds_[0].fd = m_listening_server_fd_;
  m_fds_[0].events = POLL_IN;
}

/**
 * Method to connect client and db,
 * create a pair of client's fildes and DB's fildes
 * @param fd fildes of new accepted client
 */
void tcp::Server::ConnectToDB_(int fd) {
  auto addr = tcp::NetHandler::InitAddr(DB_PORT);

  int db_fd = tcp::NetHandler::Socket(AF_INET, SOCK_STREAM, 0);
  tcp::NetHandler::IPConverter(AF_INET, DB_IP, &addr.sin_addr);
  tcp::NetHandler::Connect(db_fd, (sockaddr*)&addr, sizeof(addr));

  m_fds_[m_fds_counter_].fd = fd;
  m_fds_[m_fds_counter_++].events = POLLIN;
  m_fds_[m_fds_counter_].fd = db_fd;
  m_fds_[m_fds_counter_++].events = POLLIN;

  m_clients_.push_back(Client(fd, db_fd, &m_fds_[m_fds_counter_ - 2],
                              &m_fds_[m_fds_counter_ - 1]));

  std::cout << "Added new pair of client and DB" << std::endl;
}

/**
 * Method for accepting new users,
 * calls ConnectToDB_() method after accepts
 * incoming request to connect
 */
void tcp::Server::AddNewUsers_() {
  int new_fd = 0;
  std::cout << "\nGot the request to add new user\n";
  do {
    new_fd = accept(m_listening_server_fd_, NULL, NULL);
    if (new_fd < 0) {
      if (errno != EWOULDBLOCK) {
        perror("ERROR: Accept failed ");
        m_shutdown_server_ = true;
      }
      break;
    }
    ConnectToDB_(new_fd);
  } while (new_fd != -1);
}

tcp::Server::~Server() {
  std::cout << "destructor";
  delete m_logger_;
  close(m_listening_server_fd_);
  for (auto it : m_clients_) {
    close(it.GetFd());
    close(it.GetDBFd());
  }
}

/**
 * Sends a packet of data from user to DB,
 * calls CreateLog() method if request marked as an SQL request
 * @param client the one client that has request to DB
 */
void tcp::Server::FromUser_(tcp::Client& client) {
  char buf[BUFF_LEN]{};
  bool close_connection = false;
  auto status = tcp::NetHandler::Read(client.GetFd(), buf, BUFF_LEN);
  if (status.second) close_connection = true;

  if (buf[4] == 3 || buf[4] == 22) {
    try {
      m_logger_->CreateLog(buf + 5);
    } catch (std::invalid_argument const& ex) {
      std::cout << ex.what();
    }
  }

  status = tcp::NetHandler::Write(client.GetDBFd(), buf, status.first);
  if (status.second) close_connection = true;

  // if something went wrong - close both connections
  if (close_connection) {
    DisconnectUser_(client);
    m_compress_arr_ = true;
  }
}

/**
 * Sends a packet of data from DB to user,
 * @param client the one client that has something to get from DB
 */
void tcp::Server::ToUser_(tcp::Client& client) {
  char buf[BUFF_LEN]{};
  bool close_connection = false;
  auto status = tcp::NetHandler::Read(client.GetDBFd(), buf, BUFF_LEN);
  if (status.second) close_connection = true;
  status = tcp::NetHandler::Write(client.GetFd(), buf, status.first);
  if (status.second) close_connection = true;

  // if something went wrong - close both connections
  if (close_connection) {
    DisconnectUser_(client);
    m_compress_arr_ = true;
  }
}

/**
 * Disconnects user from server,
 * closes client's fildes and the DB's fildes
 * @param client the one client that has to be disconnected
 */
void tcp::Server::DisconnectUser_(Client& client) {
  close(client.GetFd());
  close(client.GetDBFd());
  client.SetFd(-1);
  client.SetDBFd(-1);
}

/**
 * processes the events from every fildes,
 * calls DisconnectUser() when got the request to disconnect
 * if client has smth calls - FromUser()
 * if DB has smth - calls ToUser()
 */
void tcp::Server::EventsProcessing_() {
  if (m_fds_[0].revents == POLLIN) AddNewUsers_();
  for (Client curr_client : m_clients_) {
    auto events = curr_client.GetEvents();
    // there is no requests from DB neither from client
    if (events.first == 0 && events.second == 0) {
      continue;
      // request to disconnect
    } else if (events.first == SIGSTOP || events.second == SIGSTOP) {
      DisconnectUser_(curr_client);
      // ready to be read from user
    } else if (events.first == POLL_IN) {
      FromUser_(curr_client);
      // ready to be read from DB
    } else if (events.second == POLLIN) {
      ToUser_(curr_client);
    }
  }
  std::remove_if(m_clients_.begin(), m_clients_.end(),
                 [](Client client) { return client.GetFd() == -1; });
}

/**
 * Compresses the array of pollfd structures
 */
void tcp::Server::CompressArray_() {
  m_compress_arr_ = false;
  for (int i = 0; i < m_fds_counter_; ++i) {
    if (m_fds_[i].fd == -1) {
      for (int j = i; j < m_fds_counter_; ++j) {
        m_fds_[j].fd = m_fds_[j + 1].fd;
      }
      --i;
      --m_fds_counter_;
    }
  }
}

/**
 * Main method that calls poll()
 * if any client has any request calls EventProcessing()
 */
void tcp::Server::HandlingCycle() {
  int status = 0;
  do {
    // setting poll timeout to infinite
    status = poll(m_fds_, m_fds_counter_, -1);
    if (status < 0) {
      perror("ERROR: Poll fails while waiting for the request ");
      break;
    } else if (status == 0) {
      perror("ERROR: Poll timed out ");
      break;
    }
    EventsProcessing_();
    if (m_compress_arr_) {
      CompressArray_();
    }
  } while (!m_shutdown_server_);
}

void tcp::Server::SigHandler(int signum) {
  std::cout << "\nSig exit" << std::endl;
  (void)signum;
  m_shutdown_server_ = false;
}

int main(int argc, char** argv) {
  if (argc == 4) {
    tcp::Server server(atoi(argv[1]), argv[2], atoi(argv[3]));
    signal(SIGINT, tcp::Server::SigHandler);
    server.HandlingCycle();
  } else {
    std::cout << "You'd better type like this:\n\
    make server SERV_PORT='port' DB_IP='0.0.0.0' DB_PORT=3306\n";
  }
  return 0;
}