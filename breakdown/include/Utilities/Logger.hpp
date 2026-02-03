#pragma once

#include <print>
#include <source_location>
#include <string_view>
#include <cstdio>
#include <format>
#include <utility>
#include <atomic>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace logger
{
    enum class LogLevel
    {
        Info,       // We'll use Green
        Warning,    // We'll use yellow
        Error,      // We'll use red
        None
    };
    namespace Color
    {
        constexpr std::string_view Reset = "\033[0m"; // Use for in-message reset
        constexpr std::string_view Red = "\033[31m";
        constexpr std::string_view Green = "\033[32m";
        constexpr std::string_view Yellow = "\033[33m";
        constexpr std::string_view Blue = "\033[34m";
        constexpr std::string_view Magenta = "\033[35m";
        constexpr std::string_view Cyan = "\033[36m";
        constexpr std::string_view White = "\033[37m";
    }
    struct LogEntry
    {
        std::string message;
        std::source_location location;
        LogLevel level;
    };
    // format file path to just the filename instead of printing the absolute path
    constexpr std::string_view formatPath(std::string_view path) {
        auto lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string_view::npos)
        {
            return path.substr(lastSlash + 1);
        }

        return path;
    }

    // the detail namespace is for the logger's async worker
    namespace detail
    {
        class LogWorker
        {
        public:
            LogWorker() : logWorker(&LogWorker::processLogs, this) { /* empty */ }
            ~LogWorker() {
                stopFlag = true;
                logC_V.notify_all();
                logWorker.join();
            }

            void push(LogEntry entry) {
                {
                    std::scoped_lock lock(logMutex);
                    logQueue.push(std::move(entry));
                }
                logC_V.notify_one();
            }

        private:
            void processLogs() {
                while (true)
                {
                    LogEntry entry{};
                    {
                        std::unique_lock<std::mutex> lock(logMutex);
                        logC_V.wait(lock, [this](){ return stopFlag || !logQueue.empty(); });
                        if (stopFlag && logQueue.empty())
                        {
                            break;
                        }
                        if (!logQueue.empty())
                        {
                            entry = std::move(logQueue.front());
                            logQueue.pop();
                        }
                    }

                    std::string_view colorStr{};
                    std::string_view levelStr{};

                    switch (entry.level)
                    {
                        case LogLevel::Error:
                            colorStr = Color::Red;
                            levelStr = "ERROR";
                            break;
                        case LogLevel::Warning:
                            colorStr = Color::Yellow;
                            levelStr = "WARNING";
                            break;
                        case LogLevel::Info:
                            colorStr = Color::Green;
                            levelStr = "INFO";
                            break;
                        default:
                            // something went wrong
                            colorStr = Color::White;
                            levelStr = "UNKNOWN";
                            break;
                    }

                    // Select appropriate stream (stderr for error, stdout for others)
                    FILE* stream = (entry.level == LogLevel::Error) ? stderr : stdout;

                    // Prepare the format string
                    constexpr auto fmtString = "[[{}{}{}]] {}({}:{}) --> {}{}{}";
                    // [[Error]] file: file_name(line:column) 'function_name' --> message
                    std::println(stream, fmtString,
                        colorStr,
                        levelStr,
                        Color::Reset,
                        formatPath(entry.location.file_name()),
                        entry.location.line(),
                        entry.location.column(),
                        //loc.function_name(), // too verbose but will leave here for debug
                        colorStr,
                        entry.message,
                        Color::Reset
                    );
                }
            }

        private:
            std::queue<LogEntry> logQueue;
            std::atomic<bool> stopFlag{ false };
            std::mutex logMutex;
            std::condition_variable logC_V;
            std::jthread logWorker;
        };

        inline LogWorker& getWorker()
        {
            static LogWorker worker;
            return worker;
        }
    }

    // Option for modifying log level
    #ifdef NDEBUG
        // uncomment the line beneath to restrict release builds to print only Error logs
        // inline LogLevel currentLevel = LogLevel::Error;
        // if you uncomment the line above, then comment the line below
        inline LogLevel currentLevel = LogLevel::Info;
    #else
    // For non-release builds, we'll print Warning and Info logs too
        inline LogLevel currentLevel = LogLevel::Info;
    #endif

    inline void setLevel(LogLevel level)
    {
        currentLevel = level;
    }

    inline void forceVerbose()
    {
        currentLevel = LogLevel::Info;
    }

    inline void Print(LogLevel level, std::string_view message, const std::source_location& loc)
    {
        // Only print current level and above
        if (level < currentLevel)
        {
            return;
        }

        LogEntry entry {
            .message = std::string(message),
            .location = loc,
            .level = level
        };

        detail::getWorker().push(std::move(entry));
    }

    inline void Info(std::string_view message,
        const std::source_location& loc = std::source_location::current())
    {
        Print(LogLevel::Info, message, loc);
    }

    inline void Warn(std::string_view message,
        const std::source_location& loc = std::source_location::current())
    {
        Print(LogLevel::Warning, message, loc);
    }

    inline void Error(std::string_view message,
        const std::source_location& loc = std::source_location::current())
    {
        Print(LogLevel::Error, message, loc);
    }
}
