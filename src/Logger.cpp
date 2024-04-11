#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <cstdarg>

#include <Logger.h>

Logger* Logger::m_pInstance = nullptr;

Logger& Logger::init(const std::string& logFile)
{
    static std::unique_ptr<Logger> instance(new Logger(logFile));
    m_pInstance = instance.get();

    return *m_pInstance;
}

Logger& Logger::getInstance(const std::string& logFile)
{
    if (!m_pInstance)
    {
        throw std::runtime_error("Logger is not initialized, must call init() before accessing through getInstance()");
    }

    return *m_pInstance;
}

void Logger::logString(const std::string& format, ...)
{
    if (!m_file.is_open()) {
        std::cerr << "Log m_file is not open\n";
        std::cout << "Log file is not open\n";
        return;
    }

    // TODO: add timestamp

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
    va_end(args);

    m_file << buffer << "\n";
}