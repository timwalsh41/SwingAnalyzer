#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <fstream>

class Logger
{
    public:
        static Logger& init(const std::string& logFile);
        static Logger& getInstance(const std::string& logFile);
        void logString(const std::string& format, ...);

    private:
        std::ofstream m_file;
        static Logger* m_pInstance;

        Logger(const std::string& logFile) 
        {
            m_file.open(logFile);
            if (!m_file.is_open()) 
            {
                std::cerr << "Failed to open log file: " << logFile << std::endl;
            }

            std::cout << "Opened log file\n";
        }

    protected:
        friend class std::default_delete<Logger>;

        ~Logger() 
        {
            if (m_file.is_open()) {
                m_file.close();
            }
            std::cout << "Closing logger\n";
        }
};

#endif // LOGGER_H