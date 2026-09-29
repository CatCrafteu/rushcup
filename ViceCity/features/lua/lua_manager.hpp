#pragma once

#include <sol/sol.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>
#include <filesystem>
#include "../../core/sdk/structs.hpp"
#include "../../core/sdk/interfaces.hpp"
#include "../../core/config/config_manager.hpp"
#include "../../core/logger/logger.hpp"

namespace features::lua {

class LuaManager {
public:
    static LuaManager& Instance() {
        static LuaManager instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    
    // Script management
    bool LoadScript(const std::string& name);
    bool UnloadScript(const std::string& name);
    bool ReloadScript(const std::string& name);
    bool EnableScript(const std::string& name, bool enable);
    bool SetAutoLoad(const std::string& name, bool autoLoad);
    
    std::vector<std::string> GetLoadedScripts() const;
    std::vector<std::string> GetAvailableScripts() const;
    std::optional<std::string> GetScriptContent(const std::string& name);
    bool SaveScript(const std::string& name, const std::string& content);
    bool CreateScript(const std::string& name, const std::string& content = "");
    bool DeleteScript(const std::string& name);
    
    // Script info
    struct ScriptInfo {
        std::string name;
        std::string path;
        bool loaded = false;
        bool enabled = false;
        bool autoLoad = false;
        bool hasError = false;
        std::string errorMessage;
        std::string author;
        std::string description;
        std::string version;
        std::chrono::system_clock::time_point lastModified;
        size_t size = 0;
    };
    
    std::optional<ScriptInfo> GetScriptInfo(const std::string& name);
    
    // Execution
    bool ExecuteString(const std::string& code, std::string* output = nullptr);
    bool ExecuteFile(const std::string& path, std::string* output = nullptr);
    
    // Callbacks
    using LuaCallback = std::function<void(sol::state&)>;
    void RegisterCallback(const std::string& event, LuaCallback callback);
    void UnregisterCallback(const std::string& event);
    
    // Built-in events
    void OnCreateMove(sdk::CUserCmd* cmd);
    void OnFrameStageNotify(int stage);
    void OnRender();
    void OnFireEvent(void* event);
    void OnDispatchSound(void* sound);
    void OnUnload();
    void OnMenuOpen(bool open);
    
    // Console
    struct ConsoleEntry {
        enum class Type { Log, Warning, Error, System, Input } type;
        std::string message;
        float time = 0.0f;
    };
    
    void AddConsoleEntry(ConsoleEntry::Type type, const std::string& message);
    std::vector<ConsoleEntry> GetConsoleEntries() const;
    void ClearConsole();
    
    // State
    sol::state& GetState() { return m_lua; }
    bool IsInitialized() const { return m_initialized; }
    
    // Sandbox
    void SetSandboxEnabled(bool enabled) { m_sandboxEnabled = enabled; }
    bool IsSandboxEnabled() const { return m_sandboxEnabled; }
    
    // Memory limit
    void SetMemoryLimit(size_t bytes) { m_memoryLimit = bytes; }
    size_t GetMemoryLimit() const { return m_memoryLimit; }
    
    // Timeout
    void SetTimeout(int ms) { m_timeoutMs = ms; }
    int GetTimeout() const { return m_timeoutMs; }

private:
    LuaManager() = default;
    ~LuaManager() = default;
    
    sol::state m_lua;
    std::unordered_map<std::string, std::unique_ptr<sol::state>> m_scripts;
    std::unordered_map<std::string, std::vector<LuaCallback>> m_callbacks;
    std::vector<ConsoleEntry> m_console;
    size_t m_consoleMaxEntries = 1000;
    mutable std::mutex m_mutex;
    
    bool m_initialized = false;
    bool m_sandboxEnabled = true;
    size_t m_memoryLimit = 64 * 1024 * 1024; // 64 MB
    int m_timeoutMs = 5000;
    
    std::string m_scriptsPath = "ViceCity/lua/scripts/";
    std::string m_libsPath = "ViceCity/lua/libs/";
    
    void InitializeLuaState();
    void RegisterAPI();
    void LoadBuiltinScripts();
    void LoadAutoLoadScripts();
    void SetupSandbox();
    void SetupMemoryLimit();
    void SetupTimeout();
    
    // API registration
    void RegisterMenuAPI();
    void RegisterEngineAPI();
    void RegisterEntityAPI();
    void RegisterRenderAPI();
    void RegisterCallbackAPI();
    void RegisterUtilityAPI();
    void RegisterConfigAPI();
    void RegisterLoggerAPI();
    void RegisterMathAPI();
    void RegisterInputAPI();
    void RegisterHookAPI();
    
    // Error handling
    static int LuaPanicHandler(lua_State* L);
    static int LuaErrorHandler(lua_State* L);
    void HandleLuaError(const std::string& context, const std::string& error);
    
    // Script loading
    std::string ReadScriptFile(const std::string& name);
    bool ValidateScript(const std::string& content);
    
    // Console callbacks
    void PrintCallback(const std::string& msg);
    void WarningCallback(const std::string& msg);
    void ErrorCallback(const std::string& msg);
};

} // namespace features::lua