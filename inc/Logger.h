#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <mutex>
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
    static Logger &get()
    {
        // force this call once so that we don't create multiple instances
        std::call_once(m_once_flag, [] {
            static std::unique_ptr<Logger> instance(new Logger());
            m_pInstance = instance.get();
        });

        return *m_pInstance;
    }

    void logString(LogSeverity severity, const std::string &format, ...);
    void setLogLevel(LogSeverity severity)
    {
        m_log_level = severity;
    }

    template <typename... Args> void debug(const std::string &format, Args... args)
    {
        logString(DEBUG, format, std::forward<Args>(args)...);
    }

    template <typename... Args> void info(const std::string &format, Args... args)
    {
        logString(INFO, format, std::forward<Args>(args)...);
    }

    template <typename... Args> void warn(const std::string &format, Args... args)
    {
        logString(WARN, format, std::forward<Args>(args)...);
    }

    template <typename... Args> void error(const std::string &format, Args... args)
    {
        logString(ERROR, format, std::forward<Args>(args)...);
    }

    template <typename... Args> void fatal(const std::string &format, Args... args)
    {
        logString(FATAL, format, std::forward<Args>(args)...);
    }

  private:
    std::ofstream m_file;
    static Logger *m_pInstance;
    LogSeverity m_log_level = DEBUG; // TODO: set to INFO

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

        info("Opened log file %s", log_filename.str().c_str());
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
    std::mutex m_mutex;
    static std::once_flag m_once_flag;

    const std::unordered_map<LogSeverity, std::string> m_log_severity_map = {
        {DEBUG, "DEBUG"}, {INFO, "INFO"}, {WARN, "WARNING"}, {ERROR, "ERROR"}, {FATAL, "FATAL"}};
};

#endif // LOGGER_H