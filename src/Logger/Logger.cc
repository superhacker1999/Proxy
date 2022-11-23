#include "Logger.h"

/**
 * Logger contructor
 * opens a file to log in
 * prints an error if cant open file
 * @param file_path file path to the log file that will be used
 */
Logger::Logger(const char* file_path) {
  m_file_.open(file_path, std::fstream::app);
  if (!m_file_.is_open()) {
    std::cout << "Couldn't open file for writing.\n";
  }
}

Logger::~Logger() { m_file_.close(); }

/**
 * adds a new log data to file
 * @param buffer SQL request that came from client
 */
void Logger::CreateLog(const char* buffer) {
  if (m_file_.is_open()) {
    auto now = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(now);
    m_file_ << "SQL reuest: " << std::ctime(&end_time) << buffer << "\n\n";
  } else {
    std::cout
        << "Couldn't write logs\nPlease rerun server with existing log file.\n";
  }
}