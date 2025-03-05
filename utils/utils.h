#ifndef UTILS_HEADER_H
#define UTILS_HEADER_H
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <ctime>
#include <mutex>
#include <thread>
#include <sstream>

namespace Utils
{
    using namespace std;

    inline std::string execCommand(const std::string &cmd)
    {
        std::array<char, 128> buffer;
        std::string result;

        // Define the deleter type explicitly
        using PipeDeleter = std::function<int(FILE *)>;

        // Use a lambda to ensure proper cleanup
        std::unique_ptr<FILE, PipeDeleter> pipe(popen(cmd.c_str(), "r"), pclose);

        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }

        return result;
    }
    static std::mutex exec_command_mutex; // Mutex for safe console output
    inline std::string runCommand(const std::string &command)
    {
        try
        {
            std::string output = execCommand(command);

            // Use mutex to prevent race conditions when printing
            std::lock_guard<std::mutex> lock(exec_command_mutex);
            return output;
        }
        catch (const std::exception &e)
        {
            std::lock_guard<std::mutex> lock(exec_command_mutex);
            return nullptr;
        }
    }
    inline void folder_exists(const std::string &folder_path) noexcept
    {
        if (!std::filesystem::exists(folder_path))
        {
            std::filesystem::create_directories(folder_path);
        }
    }

    // Get current timestamp
    inline std::string getTimestamp()
    {
        time_t now = time(0);
        char buffer[80];
        struct tm tstruct;
        tstruct = *localtime(&now);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %X", &tstruct);
        return buffer;
    }

    // Extract filename from full path (e.g., "/home/user/main.cpp" → "main.log")
    inline std::string extractFilename(const std::string &path)
    {
        size_t lastSlash = path.find_last_of("/\\");
        std::string filename = (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);
        size_t dotPos = filename.find_last_of('.');
        return (dotPos == std::string::npos) ? filename : filename.substr(0, dotPos);
    }

#define LOG_INFO(msg) Logger::Instance().log("INFO", msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::Instance().log("WARNING", msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::Instance().log("ERROR", msg, __FILE__, __LINE__)
    class Logger
    {
    private:
        std::ofstream logFile;
        std::mutex logMutex;  // Ensures thread safety
        std::string filename; // Stores the unique filename

        // Generate a unique filename with timestamp
        std::string generateFilename(std::string folder = "log/", std::string file = "log")
        {
            folder_exists(folder);
            time_t now = time(0);
            struct tm tstruct;
            char buffer[80];
            tstruct = *localtime(&now);
            strftime(buffer, sizeof(buffer), "_%Y-%m-%d_%H-%M-%S.log", &tstruct);
            std::string file_log = folder + file + std::string(buffer);
            return file_log;
        }

    public:
        Logger() = default;
        virtual ~Logger() = default;
        static Logger &Instance()
        {
            static Logger logger_instance;
            return logger_instance;
        }
        // Log message with level, file, line, and thread info
        void log(const std::string &level, const std::string &message, const std::string &file, int line)
        {
            std::lock_guard<std::mutex> lock(logMutex); // Thread safety

            if (!logFile.is_open())
            {
                filename = generateFilename("log/", extractFilename(file));
                logFile.open(filename, std::ios::app);
                if (!logFile)
                {
                    std::cerr << "Error: Cannot open log file!" << std::endl;
                }
                else
                {
                    std::cout << "Logging to: " << filename << std::endl;
                }
            }
            std::ostringstream logEntry;
            logEntry << "[" << getTimestamp() << "] "
                     << "[Thread " << std::this_thread::get_id() << "] "
                     << "[" << level << "] "
                     << message
                     << " (File: " << file << ", Line: " << line << ")";

            if (logFile.is_open())
            {
                logFile << logEntry.str() << std::endl;
            }
            else
            {
                std::cerr << "Error: Log file is not open!" << std::endl;
            }
        }
    };
}

#endif