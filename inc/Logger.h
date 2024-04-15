#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>

enum LogSeverity
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

class Logger
{
  public:
    static Logger &get();
    void logString(LogSeverity severity, const std::string &format, ...);
    void debug(const std::string &format, ...);
    void info(const std::string &format, ...);
    void warn(const std::string &format, ...);
    void error(const std::string &format, ...);
    void fatal(const std::string &format, ...);

  private:
    std::ofstream m_file;
    static Logger *m_pInstance;

    Logger()
    {
        auto now = std::chrono::system_clock::now();
        auto itt = std::chrono::system_clock::to_time_t(now);
        std::ostringstream log_filename;
        log_filename << "swing_analyzer " << std::put_time(std::localtime(&itt), "%Y-%m-%d %H:%M:%S") << ".log";

        m_file.open(log_filename.str());
        if (!m_file.is_open())
        {
            std::ostringstream error_string;
            error_string << "Failed to open log file: " << log_filename.str();
            throw std::runtime_error(error_string.str());
        }

        m_file << "Opened log file " << log_filename.str() << "\n";
    }

  protected:
    friend class std::default_delete<Logger>;

    ~Logger()
    {
        if (m_file.is_open())
        {
            logString(INFO, "Closing log file");
            m_file.close();
        }
    }

  private:
    const std::unordered_map<LogSeverity, std::string> m_log_severity_map = {
        {DEBUG, "DEBUG"}, {INFO, "INFO"}, {WARN, "WARNING"}, {ERROR, "ERROR"}, {FATAL, "FATAL"}};
};

#endif // LOGGER_H