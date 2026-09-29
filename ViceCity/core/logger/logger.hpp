#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>
#include <chrono>
#include <fmt/format.h>
#include <filesystem>
#include <iostream>
#include <Windows.h>

namespace core::logger {

enum class LogLevel : int {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ErrorLevel = 4,
    CRITICAL = 5,
    NONE = 6
};

struct LogEntry {
    LogLevel level;
    std::string message;
    std::string module;
    std::chrono::system_clock::time_point timestamp;
    int thread_id;
};

class Logger {
public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }
    
    bool Initialize(const std::string& logDir = "ViceCity/logs", const std::string& prefix = "ViceCity");
    void Shutdown();
    
    void SetConsoleLevel(LogLevel level) { m_consoleLevel = level; }
    void SetFileLevel(LogLevel level) { m_fileLevel = level; }
    void SetMaxFileSize(size_t bytes) { m_maxFileSize = bytes; }
    void SetMaxFiles(int count) { m_maxFiles = count; }
    void SetAsync(bool async) { m_async = async; }
    
    LogLevel GetConsoleLevel() const { return m_consoleLevel; }
    LogLevel GetFileLevel() const { return m_fileLevel; }
    
    // Main logging function
    void Log(LogLevel level, const std::string& module, const std::string& message);
    void Log(LogLevel level, const std::string& module, const char* fmt, ...);
    void Log(LogLevel level, const std::string& module, const wchar_t* fmt, ...);
    
    // Convenience functions - non-template, pass through to variadic Log
    void Trace(const std::string& module, const char* fmt, ...);
    void Debug(const std::string& module, const char* fmt, ...);
    void Info(const std::string& module, const char* fmt, ...);
    void Warn(const std::string& module, const char* fmt, ...);
    void Error(const std::string& module, const char* fmt, ...);
    void Critical(const std::string& module, const char* fmt, ...);
    
    // Simple string overloads
    void Trace(const std::string& module, const std::string& msg) { Log(LogLevel::TRACE, module, msg); }
    void Debug(const std::string& module, const std::string& msg) { Log(LogLevel::DEBUG, module, msg); }
    void Info(const std::string& module, const std::string& msg) { Log(LogLevel::INFO, module, msg); }
    void Warn(const std::string& module, const std::string& msg) { Log(LogLevel::WARN, module, msg); }
    void Error(const std::string& module, const std::string& msg) { Log(LogLevel::ErrorLevel, module, msg); }
    void Critical(const std::string& module, const std::string& msg) { Log(LogLevel::CRITICAL, module, msg); }
    
    // Module-specific loggers
    class ModuleLogger {
    public:
        ModuleLogger(const std::string& module) : m_module(module) {}
        
        void Trace(const char* fmt, ...);
        void Debug(const char* fmt, ...);
        void Info(const char* fmt, ...);
        void Warn(const char* fmt, ...);
        void Error(const char* fmt, ...);
        void Critical(const char* fmt, ...);
        
        void Trace(const std::string& msg) { Logger::Instance().Trace(m_module, msg); }
        void Debug(const std::string& msg) { Logger::Instance().Debug(m_module, msg); }
        void Info(const std::string& msg) { Logger::Instance().Info(m_module, msg); }
        void Warn(const std::string& msg) { Logger::Instance().Warn(m_module, msg); }
        void Error(const std::string& msg) { Logger::Instance().Error(m_module, msg); }
        void Critical(const std::string& msg) { Logger::Instance().Critical(m_module, msg); }
        
    private:
        std::string m_module;
    };
    
    ModuleLogger GetModuleLogger(const std::string& module) {
        return ModuleLogger(module);
    }
    
    // Flush and rotation
    void Flush();
    void RotateLogFile();
    
    // Get log entries (for UI)
    std::vector<LogEntry> GetRecentLogs(size_t count = 1000) const;
    void ClearLogs();
    
    // Console colors
    static WORD GetConsoleColor(LogLevel level);
    static const char* GetLevelString(LogLevel level);

private:
    Logger() = default;
    ~Logger() = default;
    
    void WriteToFile(const LogEntry& entry);
    void WriteToConsole(const LogEntry& entry);
    void ProcessQueue();
    std::string FormatEntry(const LogEntry& entry, bool withColor) const;
    std::string GetLogFilePath() const;
    void EnsureLogDirectory();
    void CleanOldLogs();
    
    std::string m_logDir;
    std::string m_logPrefix;
    std::ofstream m_logFile;
    std::string m_currentLogFile;
    
    LogLevel m_consoleLevel = LogLevel::INFO;
    LogLevel m_fileLevel = LogLevel::TRACE;
    size_t m_maxFileSize = 10 * 1024 * 1024; // 10 MB
    int m_maxFiles = 10;
    bool m_async = true;
    bool m_initialized = false;
    
    std::queue<LogEntry> m_logQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;
    std::thread m_workerThread;
    bool m_stopWorker = false;
    
    std::vector<LogEntry> m_recentLogs;
    mutable std::mutex m_recentLogsMutex;
    static constexpr size_t MAX_RECENT_LOGS = 5000;
    
    HANDLE m_consoleHandle = nullptr;
    bool m_consoleAllocated = false;
};

// Global macros for easy logging
#define LOG_TRACE(module, ...) core::logger::Logger::Instance().Trace(module, __VA_ARGS__)
#define LOG_DEBUG(module, ...) core::logger::Logger::Instance().Debug(module, __VA_ARGS__)
#define LOG_INFO(module, ...) core::logger::Logger::Instance().Info(module, __VA_ARGS__)
#define LOG_WARN(module, ...) core::logger::Logger::Instance().Warn(module, __VA_ARGS__)
#define LOG_ERROR(module, ...) core::logger::Logger::Instance().Error(module, __VA_ARGS__)
#define LOG_CRITICAL(module, ...) core::logger::Logger::Instance().Critical(module, __VA_ARGS__)

// Module loggers (declare in cpp)
#define DECLARE_MODULE_LOGGER(name) \
    inline core::logger::Logger::ModuleLogger g_log_##name(#name)

#define GET_MODULE_LOGGER(name) g_log_##name

// Common modules
DECLARE_MODULE_LOGGER(Core);
DECLARE_MODULE_LOGGER(Hooks);
DECLARE_MODULE_LOGGER(Memory);
DECLARE_MODULE_LOGGER(Config);
DECLARE_MODULE_LOGGER(GUI);
DECLARE_MODULE_LOGGER(Rage);
DECLARE_MODULE_LOGGER(Legit);
DECLARE_MODULE_LOGGER(Visuals);
DECLARE_MODULE_LOGGER(Misc);
DECLARE_MODULE_LOGGER(Skinchanger);
DECLARE_MODULE_LOGGER(Lua);
DECLARE_MODULE_LOGGER(Resolver);
DECLARE_MODULE_LOGGER(Netvars);

} // namespace core::logger
