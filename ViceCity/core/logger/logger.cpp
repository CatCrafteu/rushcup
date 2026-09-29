#include "core/pch.hpp"
#include "core/logger/logger.hpp"

namespace core::logger {

bool Logger::Initialize(const std::string& logDir, const std::string& prefix) {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    
    if (m_initialized) return true;
    
    // Get app data path
    char appData[MAX_PATH];
    if (SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, appData) != S_OK) {
        return false;
    }
    
    m_logDir = (std::filesystem::path(appData) / logDir).string();
    m_logPrefix = prefix;
    
    EnsureLogDirectory();
    CleanOldLogs();
    
    // Open log file
    std::string logPath = GetLogFilePath();
    m_logFile.open(logPath, std::ios::out | std::ios::app);
    if (!m_logFile.is_open()) {
        return false;
    }
    
    m_currentLogFile = logPath;
    
    // Allocate console for debug output
    if (AllocConsole()) {
        m_consoleAllocated = true;
        m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
        freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);
        SetConsoleTitleA(("ViceCity Log Console - " + prefix).c_str());
    }
    
    // Start async worker
    m_stopWorker = false;
    m_workerThread = std::thread(&Logger::ProcessQueue, this);
    
    m_initialized = true;
    Info("Logger", "Logger initialized");
    return true;
}

void Logger::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (!m_initialized) return;
        m_stopWorker = true;
    }
    m_queueCV.notify_all();
    
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    
    Flush();
    
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
    
    if (m_consoleAllocated) {
        FreeConsole();
        m_consoleAllocated = false;
    }
    
    m_initialized = false;
}

void Logger::Log(LogLevel level, const std::string& module, const std::string& message) {
    if (level < m_consoleLevel && level < m_fileLevel) return;
    
    LogEntry entry;
    entry.level = level;
    entry.message = message;
    entry.module = module;
    entry.timestamp = std::chrono::system_clock::now();
    entry.thread_id = GetCurrentThreadId();
    
    if (m_async) {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_logQueue.push(entry);
        m_queueCV.notify_one();
    } else {
        WriteToConsole(entry);
        WriteToFile(entry);
    }
    
    // Store in recent logs
    {
        std::lock_guard<std::mutex> lock(m_recentLogsMutex);
        m_recentLogs.push_back(entry);
        if (m_recentLogs.size() > MAX_RECENT_LOGS) {
            m_recentLogs.erase(m_recentLogs.begin());
        }
    }
}

void Logger::Log(LogLevel level, const std::string& module, const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Log(level, module, buffer);
}

void Logger::Log(LogLevel level, const std::string& module, const wchar_t* fmt, ...) {
    wchar_t buffer[4096];
    va_list args;
    va_start(args, fmt);
    vswprintf_s(buffer, sizeof(buffer) / sizeof(wchar_t), fmt, args);
    va_end(args);
    
    // Convert to UTF-8
    int size = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &str[0], size, nullptr, nullptr);
    if (!str.empty() && str.back() == '\0') str.pop_back();
    
    Log(level, module, str);
}

void Logger::Trace(const std::string& module, const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Log(LogLevel::TRACE, module, buffer);
}

void Logger::Debug(const std::string& module, const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Log(LogLevel::DEBUG, module, buffer);
}

void Logger::Info(const std::string& module, const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Log(LogLevel::INFO, module, buffer);
}

void Logger::Warn(const std::string& module, const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Log(LogLevel::WARN, module, buffer);
}

void Logger::Error(const std::string& module, const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Log(LogLevel::ErrorLevel, module, buffer);
}

void Logger::Critical(const std::string& module, const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Log(LogLevel::CRITICAL, module, buffer);
}

void Logger::WriteToConsole(const LogEntry& entry) {
    if (entry.level < m_consoleLevel) return;
    
    if (m_consoleHandle) {
        WORD color = GetConsoleColor(entry.level);
        SetConsoleTextAttribute(m_consoleHandle, color);
    }
    
    std::string formatted = FormatEntry(entry, true);
    std::cout << formatted << std::endl;
    
    if (m_consoleHandle) {
        SetConsoleTextAttribute(m_consoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
}

void Logger::WriteToFile(const LogEntry& entry) {
    if (entry.level < m_fileLevel) return;
    if (!m_logFile.is_open()) return;
    
    std::string formatted = FormatEntry(entry, false);
    m_logFile << formatted << std::endl;
    
    // Check rotation
    m_logFile.flush();
    if (m_logFile.tellp() > static_cast<std::streamoff>(m_maxFileSize)) {
        RotateLogFile();
    }
}

void Logger::ProcessQueue() {
    while (!m_stopWorker) {
        LogEntry entry;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_queueCV.wait_for(lock, std::chrono::milliseconds(100), [this] {
                return m_stopWorker || !m_logQueue.empty();
            });
            
            if (m_stopWorker && m_logQueue.empty()) break;
            if (m_logQueue.empty()) continue;
            
            entry = m_logQueue.front();
            m_logQueue.pop();
        }
        
        WriteToConsole(entry);
        WriteToFile(entry);
    }
}

std::string Logger::FormatEntry(const LogEntry& entry, bool withColor) const {
    auto time = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    ss << " [" << GetLevelString(entry.level) << "]";
    ss << " [" << entry.module << "]";
    ss << " " << entry.message;
    
    return ss.str();
}

std::string Logger::GetLogFilePath() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << m_logPrefix << "_";
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d");
    ss << ".log";
    return (std::filesystem::path(m_logDir) / ss.str()).string();
}

void Logger::EnsureLogDirectory() {
    std::filesystem::create_directories(m_logDir);
}

void Logger::CleanOldLogs() {
    if (!std::filesystem::exists(m_logDir)) return;
    
    std::vector<std::filesystem::path> logs;
    for (const auto& entry : std::filesystem::directory_iterator(m_logDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".log") {
            logs.push_back(entry.path());
        }
    }
    
    std::sort(logs.begin(), logs.end(), 
        [](const auto& a, const auto& b) {
            return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
        });
    
    for (size_t i = m_maxFiles; i < logs.size(); ++i) {
        std::filesystem::remove(logs[i]);
    }
}

void Logger::RotateLogFile() {
    if (!m_logFile.is_open()) return;
    
    m_logFile.close();
    CleanOldLogs();
    
    std::string logPath = GetLogFilePath();
    m_logFile.open(logPath, std::ios::out | std::ios::app);
    m_currentLogFile = logPath;
}

void Logger::Flush() {
    if (m_logFile.is_open()) {
        m_logFile.flush();
    }
    std::cout.flush();
}

std::vector<LogEntry> Logger::GetRecentLogs(size_t count) const {
    std::lock_guard<std::mutex> lock(m_recentLogsMutex);
    size_t start = m_recentLogs.size() > count ? m_recentLogs.size() - count : 0;
    return std::vector<LogEntry>(m_recentLogs.begin() + start, m_recentLogs.end());
}

void Logger::ClearLogs() {
    std::lock_guard<std::mutex> lock(m_recentLogsMutex);
    m_recentLogs.clear();
}

WORD Logger::GetConsoleColor(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // Cyan
        case LogLevel::DEBUG: return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY; // White
        case LogLevel::INFO: return FOREGROUND_GREEN | FOREGROUND_INTENSITY; // Green
        case LogLevel::WARN: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // Yellow
        case LogLevel::ErrorLevel: return FOREGROUND_RED | FOREGROUND_INTENSITY; // Red
        case LogLevel::CRITICAL: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // Magenta
        default: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // White
    }
}

const char* Logger::GetLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ErrorLevel: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

void Logger::ModuleLogger::Trace(const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Logger::Instance().Trace(m_module, buffer);
}

void Logger::ModuleLogger::Debug(const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Logger::Instance().Debug(m_module, buffer);
}

void Logger::ModuleLogger::Info(const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Logger::Instance().Info(m_module, buffer);
}

void Logger::ModuleLogger::Warn(const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Logger::Instance().Warn(m_module, buffer);
}

void Logger::ModuleLogger::Error(const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Logger::Instance().Error(m_module, buffer);
}

void Logger::ModuleLogger::Critical(const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Logger::Instance().Critical(m_module, buffer);
}

} // namespace core::logger
