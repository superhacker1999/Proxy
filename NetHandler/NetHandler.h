#ifndef SRC_NETHANDLER_NETHANDLER_H_
#define SRC_NETHANDLER_NETHANDLER_H_

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

constexpr size_t BUFF_LEN = 512;

namespace tcp {
class NetHandler {
 public:
  static int Socket(int domain, int type, int protocol);
  static void Bind(int sockfd, const sockaddr* addr, socklen_t addrlen);
  static sockaddr_in InitAddr(int port);
  static void Listen(int socket, int backlog);
  static void MakeSocketReuseable(int fd);
  static void SetNonBlockingSocket(int fd);
  static void IPConverter(int af, const char* src, void* dst);
  static void Connect(int socket, const sockaddr* address, socklen_t addr_len);
  static std::pair<int, bool> Write(int fidles, const char* buf, size_t nbyte);
  static std::pair<int, bool> Read(int fidles, char* buf, size_t nbyte);
};
}  // namespace tcp

#endif  // SRC_NETHANDLER_NETHANDLER_H_