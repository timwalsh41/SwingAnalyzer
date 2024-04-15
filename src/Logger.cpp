#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <cstdarg>

#include <Logger.h>

Logger* Logger::m_pInstance = nullptr;

Logger& Logger::get()
{
    if (!m_pInstance)
    {
        static std::unique_ptr<Logger> instance(new Logger());
        m_pInstance = instance.get();
    }

    return *m_pInstance;
}

void Logger::logString(LogSeverity severity, const std::string& format, ...)
{
    if (!m_file.is_open()) {
        std::cerr << "Log m_file is not open\n";
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto itt = std::chrono::system_clock::to_time_t(now);
    m_file << std::put_time(std::localtime(&itt), "%Y-%m-%d %H:%M:%S") << " [" << m_log_severity_map.at(severity) << "] ";

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
    va_end(args);

    m_file << buffer << std::endl;
}

// TODO: implement argument forwarding

/*
void Logger::debug(const std::string &format, ...)
{
    logString(DEBUG, ...);
}

void info(const std::string &format, ...)
{
    logString(INFO, ...);
}

void warn(const std::string &format, ...)
{
    logString(WARN, ...);
}

void error(const std::string &format, ...)
{
    logString(ERROR, ...);
}

void fatal(const std::string &format, ...)
{
    logString(FATAL, ...);
}
*/