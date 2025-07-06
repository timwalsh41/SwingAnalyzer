#include <Logger.h>

#include <cstdarg>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

Logger *Logger::m_pInstance = nullptr;
std::once_flag Logger::m_once_flag;

void Logger::logString(LogSeverity severity, const std::string &format, ...)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_file.is_open())
    {
        std::cerr << "Log m_file is not open\n";
        return;
    }

    if (severity < m_log_level)
    {
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto itt = std::chrono::system_clock::to_time_t(now);
    m_file << std::put_time(std::localtime(&itt), "%Y-%m-%d %H:%M:%S") << " [" << m_log_severity_map.at(severity)
           << "] ";

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
    va_end(args);

    m_file << buffer << std::endl;
}