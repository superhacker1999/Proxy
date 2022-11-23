#ifndef SRC_LOGGER_LOGGER_H_
#define SRC_LOGGER_LOGGER_H_

#include <string.h>

#include <chrono>
#include <fstream>
#include <iostream>

class Logger {
 private:
  std::fstream m_file_;

 public:
  Logger(const char* file_path);
  ~Logger();
  void CreateLog(const char* buffer);
};  // class Logger

#endif  // SRC_LOGGER_LOGGER_H_