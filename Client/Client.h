#ifndef SRC_CLIENT_CLIENT_H_
#define SRC_CLIENT_CLIENT_H_

#include <string.h>

#include "../NetHandler/NetHandler.h"

namespace tcp {
class Client {
 private:
  int m_fd_;     // fildes of client
  int m_db_fd_;  // fildes of DB connected with client
  pollfd* m_poll_;
  pollfd* m_db_poll_;

 public:
  Client(int own_fd, int db_fd, pollfd* client_poll, pollfd* db_poll);
  ~Client();
  int GetFd();
  void SetFd(int new_fd);
  int GetDBFd();
  void SetDBFd(int new_fd);
  std::pair<short, short> GetEvents();
};  // class Client
}  // namespace tcp

#endif  // SRC_CLIENT_CLIENT_H_