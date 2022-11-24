#include "NetHandler.h"

/**
 * A wrapper for original socket()
 * Handles errors
 * @return result of socket() call if its not an error
 */
int tcp::NetHandler::Socket(int domain, int type, int protocol) {
  int res = socket(domain, type, protocol);
  if (res == -1) {
    perror("ERROR: Couldnt get descriptor ");
    exit(EXIT_FAILURE);
  }
  return res;
}

/**
 * A wrapper for original bind()
 * Handles errors
 */
void tcp::NetHandler::Bind(int sockfd, const sockaddr* addr,
                           socklen_t addrlen) {
  if (bind(sockfd, addr, addrlen) == -1) {
    perror("ERROR: Couldnt bind socket ");
    exit(EXIT_FAILURE);
  }
}

/**
 * A wrapper for original listen()
 * Handles errors
 */
void tcp::NetHandler::Listen(int socket, int backlog) {
  if (listen(socket, backlog) == -1) {
    perror("ERROR: Couldnt start to listen ");
    exit(EXIT_FAILURE);
  }
}

/**
 * Makes a socket reuseable, a wrapper for original setsockopt()
 * Handles errors
 */
void tcp::NetHandler::MakeSocketReuseable(int fd) {
  int on = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) {
    perror("ERROR: Couldnt make socket reuseable ");
    exit(EXIT_FAILURE);
  }
}

/**
 * Makes socket to be nonblocking
 * Handles errors
 */
void tcp::NetHandler::SetNonBlockingSocket(int fd) {
  int on = 1;
  if (ioctl(fd, FIONBIO, (char*)&on) < 0) {
    perror("ERROR: Couldnt set socket to be non-blocking ");
    exit(EXIT_FAILURE);
  }
}

/**
 * A wrapper for original connect()
 * Handles errors
 */
void tcp::NetHandler::Connect(int socket, const sockaddr* address,
                              socklen_t addr_len) {
  if (connect(socket, address, addr_len) == -1) {
    perror("ERROR: Connection failed ");
    exit(EXIT_FAILURE);
  }
}

/**
 * Applies an address from string to an addr_in structure
 * by pointer
 */
void tcp::NetHandler::IPConverter(int af, const char* src, void* dst) {
  int res = inet_pton(af, src, dst);
  if (res == 0) {
    printf(
        "inet_pton failed: src does not contain a character"
        " string representing a valid network address in the specified"
        " address family\n");
    exit(EXIT_FAILURE);
  }
  if (res == -1) {
    perror("inet_pton failed ");
    exit(EXIT_FAILURE);
  }
}

/**
 * A wrapper for original write()
 * Handles errors
 * @return Pair that contains status of a write() call and a flag to close
 * connection with this client
 */
std::pair<int, bool> tcp::NetHandler::Write(int fidles, const char* buf,
                                            size_t nbyte) {
  bool close_connection = false;
  int status = write(fidles, buf, nbyte);
  if (status < 0) {
    if (errno != EWOULDBLOCK) {
      perror("ERROR: error while reading request ");
      close_connection = true;
    }
  } else if (status == 0) {
    std::cout << "Connection close by peer with fd number = " << fidles
              << std::endl;
    close_connection = true;
  }
  return {status, close_connection};
}

/**
 * A wrapper for original write()
 * Handles errors
 * @return Pair that contains how much bytes was received and a flag to close
 * connection with this client
 */
std::pair<int, bool> tcp::NetHandler::Read(int fidles, char* buf,
                                           size_t nbyte) {
  bool close_connection = false;
  int status = read(fidles, buf, nbyte);
  if (status < 0) {
    perror("ERROR: Couldnt read message from FD ");
    close_connection = true;
  }
  return {status, close_connection};
}

/**
 * Initialises a sockaddr_in structure
 * @param port port that has to be connected
 * @return sockaddr_in structure filled with a new data
 */
sockaddr_in tcp::NetHandler::InitAddr(int port) {
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  return addr;
}
