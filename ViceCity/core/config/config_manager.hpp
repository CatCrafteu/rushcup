#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <memory>

namespace core::config {

using json = nlohmann::json;

class ConfigManager {
public:
    struct ConfigSchema {
        int version = 1;
        std::string name;
        std::string description;
        json schema;
    };
    
    struct ConfigMetadata {
        std::string name;
        std::string author;
        std::string description;
        int version = 1;
        std::string created;
        std::string modified;
        std::string mapName;
        bool isAutoSave = false;
        bool isCloudSynced = false;
    };

    static ConfigManager& Instance() {
        static ConfigManager instance;
        return instance;
    }
    
    bool Initialize(const std::string& configDir = "ViceCity");
    void Shutdown();
    
    // Config loading/saving
    bool Load(const std::string& name);
    bool Save(const std::string& name);
    bool SaveAs(const std::string& name);
    bool Delete(const std::string& name);
    bool Rename(const std::string& oldName, const std::string& newName);
    
    // Config listing
    std::vector<std::string> ListConfigs() const;
    std::vector<std::string> ListConfigsForMap(const std::string& map) const;
    
    // Current config
    const std::string& GetCurrentConfig() const { return m_currentConfig; }
    void SetCurrentConfig(const std::string& name) { m_currentConfig = name; }
    
    // Auto-save
    void EnableAutoSave(bool enable, int intervalMs = 5000);
    void DisableAutoSave() { EnableAutoSave(false); }
    void TriggerAutoSave();
    
    // Per-map configs
    void SetMapConfig(const std::string& map, const std::string& config);
    std::optional<std::string> GetMapConfig(const std::string& map) const;
    
    // Import/Export
    std::string ExportToBase64(const std::string& name) const;
    std::string ExportToBase64Current() const;
    bool ImportFromBase64(const std::string& name, const std::string& base64);
    bool ImportFromBase64Current(const std::string& base64);
    bool ExportToFile(const std::string& name, const std::string& path) const;
    bool ImportFromFile(const std::string& name, const std::string& path);
    bool ExportToClipboard(const std::string& name) const;
    bool ImportFromClipboard(const std::string& name);
    
    // Cloud sync (GitHub Gist)
    bool SyncToCloud(const std::string& name, const std::string& gistToken, const std::string& gistId = "");
    bool SyncFromCloud(const std::string& gistToken, const std::string& gistId);
    bool SyncAllToCloud(const std::string& gistToken);
    bool SyncAllFromCloud(const std::string& gistToken);
    void SetCloudSyncEnabled(bool enable) { m_cloudSyncEnabled = enable; }
    bool IsCloudSyncEnabled() const { return m_cloudSyncEnabled; }
    
    // JSON access
    json& GetConfig() { return m_config; }
    const json& GetConfig() const { return m_config; }
    json& GetConfig(const std::string& name);
    const json& GetConfig(const std::string& name) const;
    
    // Value access with type safety
    template<typename T>
    std::optional<T> Get(const std::string& key, const T& defaultValue = T{}) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_config.find(key);
        if (it == m_config.end()) return defaultValue;
        try {
            return it->get<T>();
        } catch (...) {
            return defaultValue;
        }
    }
    
    template<typename T>
    void Set(const std::string& key, const T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config[key] = value;
        m_modified = true;
    }
    
    template<typename T>
    std::optional<T> Get(const std::string& category, const std::string& key, const T& defaultValue = T{}) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto catIt = m_config.find(category);
        if (catIt == m_config.end()) return defaultValue;
        auto keyIt = catIt->find(key);
        if (keyIt == catIt->end()) return defaultValue;
        try {
            return keyIt->get<T>();
        } catch (...) {
            return defaultValue;
        }
    }
    
    template<typename T>
    void Set(const std::string& category, const std::string& key, const T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config[category][key] = value;
        m_modified = true;
    }
    
    // Array helpers
    template<typename T>
    std::vector<T> GetArray(const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_config.find(key);
        if (it == m_config.end() || !it->is_array()) return {};
        std::vector<T> result;
        for (const auto& item : *it) {
            try {
                result.push_back(item.get<T>());
            } catch (...) {}
        }
        return result;
    }
    
    template<typename T>
    void SetArray(const std::string& key, const std::vector<T>& values) {
        std::lock_guard<std::mutex> lock(m_mutex);
        json arr = json::array();
        for (const auto& v : values) arr.push_back(v);
        m_config[key] = arr;
        m_modified = true;
    }
    
    // Object helpers
    bool HasKey(const std::string& key) const;
    bool HasKey(const std::string& category, const std::string& key) const;
    void RemoveKey(const std::string& key);
    void RemoveKey(const std::string& category, const std::string& key);
    void ClearCategory(const std::string& category);
    
    // Config validation
    bool ValidateSchema(const json& schema);
    void ApplyDefaults(const json& defaults);
    
    // Hot reload
    void EnableHotReload(bool enable);
    bool IsHotReloadEnabled() const { return m_hotReload; }
    void CheckHotReload();
    
    // Callbacks
    using ConfigCallback = std::function<void(const std::string& name, const json& config)>;
    void OnConfigLoaded(ConfigCallback cb) { m_onLoadCallbacks.push_back(cb); }
    void OnConfigSaved(ConfigCallback cb) { m_onSaveCallbacks.push_back(cb); }
    void OnConfigChanged(ConfigCallback cb) { m_onChangeCallbacks.push_back(cb); }
    
    // Get metadata
    ConfigMetadata GetMetadata(const std::string& name) const;
    void SetMetadata(const std::string& name, const ConfigMetadata& meta);
    
    // Backup
    void CreateBackup(const std::string& name);
    std::vector<std::string> ListBackups(const std::string& name) const;
    bool RestoreBackup(const std::string& name, const std::string& backupName);
    
    // Paths
    std::string GetConfigPath(const std::string& name) const;
    std::string GetConfigDir() const { return m_configDir.string(); }
    std::string GetBackupDir() const { return m_backupDir.string(); }

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    
    std::filesystem::path m_configDir;
    std::filesystem::path m_backupDir;
    std::string m_currentConfig;
    json m_config;
    json m_schema;
    ConfigSchema m_schemaInfo;
    bool m_modified = false;
    bool m_initialized = false;
    bool m_autoSave = false;
    int m_autoSaveInterval = 5000;
    bool m_cloudSyncEnabled = false;
    bool m_hotReload = false;
    std::filesystem::file_time_type m_lastWriteTime;
    mutable std::mutex m_mutex;
    
    std::vector<ConfigCallback> m_onLoadCallbacks;
    std::vector<ConfigCallback> m_onSaveCallbacks;
    std::vector<ConfigCallback> m_onChangeCallbacks;
    
    void EnsureDirectories();
    void LoadSchema();
    void SaveSchema();
    void TriggerLoadCallbacks(const std::string& name);
    void TriggerSaveCallbacks(const std::string& name);
    void TriggerChangeCallbacks(const std::string& name);
    std::string GetCurrentTimestamp() const;
    std::string Base64Encode(const std::string& input) const;
    std::string Base64Decode(const std::string& input) const;
    bool ValidateConfig(const json& config) const;
    void MergeDefaults(json& config, const json& defaults);
};

// Config categories (for organization)
namespace categories {
    inline constexpr const char* AIMBOT_RAGE = "aimbot_rage";
    inline constexpr const char* AIMBOT_LEGIT = "aimbot_legit";
    inline constexpr const char* ANTI_AIM = "anti_aim";
    constexpr const char* EXPLOITS = "exploits";
    constexpr const char* RESOLVER = "resolver";
    constexpr const char* VISUALS_ESP = "visuals_esp";
    constexpr const char* VISUALS_CHAMS = "visuals_chams";
    constexpr const char* VISUALS_WORLD = "visuals_world";
    constexpr const char* VISUALS_RADAR = "visuals_radar";
    constexpr const char* VISUALS_EFFECTS = "visuals_effects";
    constexpr const char* MISC_MOVEMENT = "misc_movement";
    constexpr const char* MISC_LOGS = "misc_logs";
    constexpr const char* MISC_SKINCHANGER = "misc_skinchanger";
    constexpr const char* SKINCHANGER_INVENTORY = "skinchanger_inventory";
    constexpr const char* SKINCHANGER_CASES = "skinchanger_cases";
    constexpr const char* SKINCHANGER_STICKERS = "skinchanger_stickers";
    constexpr const char* SKINCHANGER_GLOVES = "skinchanger_gloves";
    constexpr const char* SKINCHANGER_AGENTS = "skinchanger_agents";
    constexpr const char* SKINCHANGER_MUSIC = "skinchanger_music";
    constexpr const char* SKINCHANGER_MEDALS = "skinchanger_medals";
    constexpr const char* CONFIG = "config";
    constexpr const char* LUA = "lua";
    constexpr const char* GUI = "gui";
    constexpr const char* COLORS = "colors";
    constexpr const char* KEYBINDS = "keybinds";
}

// Default config schema
inline const json DEFAULT_SCHEMA = R"({
    "version": 1,
    "name": "ViceCity",
    "description": "ViceCity CS2 Cheat Configuration",
    "schema": {
        "aimbot_rage": {
            "enabled": {"type": "boolean", "default": true},
            "silent_aim": {"type": "boolean", "default": true},
            "auto_scope": {"type": "boolean", "default": true},
            "auto_stop": {"type": "boolean", "default": true}
        },
        "aimbot_legit": {
            "triggerbot": {
                "enabled": {"type": "boolean", "default": false},
                "keybind": {"type": "string", "default": ""},
                "delay_min": {"type": "integer", "default": 0},
                "delay_max": {"type": "integer", "default": 50}
            },
            "backtrack": {
                "enabled": {"type": "boolean", "default": false},
                "time_limit": {"type": "integer", "default": 200}
            },
            "legit_aa": {
                "enabled": {"type": "boolean", "default": false}
            },
            "movement": {
                "bhop_enabled": {"type": "boolean", "default": false},
                "auto_strafe_enabled": {"type": "boolean", "default": false}
            }
        },
        "anti_aim": {
            "enabled": {"type": "boolean", "default": true},
            "pitch": {"type": "string", "default": "down"},
            "yaw": {"type": "string", "default": "backward"}
        },
        "exploits": {
            "double_tap": {"enabled": {"type": "boolean", "default": true}},
            "hide_shots": {"enabled": {"type": "boolean", "default": true}},
            "fake_lag": {"enabled": {"type": "boolean", "default": true}, "limit": {"type": "integer", "default": 14}}
        },
        "resolver": {
            "enabled": {"type": "boolean", "default": true},
            "log_misses": {"type": "boolean", "default": true},
            "animfix": {"enabled": {"type": "boolean", "default": true}}
        },
        "visuals_esp": {
            "enabled": {"type": "boolean", "default": true},
            "box": {"enabled": {"type": "boolean", "default": true}},
            "skeleton": {"enabled": {"type": "boolean", "default": true}}
        },
        "visuals_chams": {
            "enabled": {"type": "boolean", "default": true}
        },
        "visuals_world": {
            "grenade_preview": {"enabled": {"type": "boolean", "default": true}},
            "hitmarker": {"enabled": {"type": "boolean", "default": true}}
        },
        "visuals_radar": {
            "enabled": {"type": "boolean", "default": true}
        },
        "visuals_effects": {
            "bullet_tracers": {"enabled": {"type": "boolean", "default": true}},
            "hit_numbers": {"enabled": {"type": "boolean", "default": true}}
        },
        "misc_movement": {
            "recoil_crosshair": {"enabled": {"type": "boolean", "default": true}}
        },
        "misc_logs": {
            "hit_logs": {"type": "boolean", "default": true},
            "damage_logs": {"type": "boolean", "default": true}
        },
        "misc_skinchanger": {
            "enabled": {"type": "boolean", "default": true},
            "auto_apply": {"type": "boolean", "default": true}
        },
        "config": {
            "auto_save": {"type": "boolean", "default": true},
            "auto_save_interval": {"type": "integer", "default": 5000}
        },
        "lua": {
            "enabled": {"type": "boolean", "default": true}
        },
        "gui": {
            "menu_key": {"type": "string", "default": "INSERT"},
            "dpi_scale": {"type": "number", "default": 1.0}
        }
    }
})"_json;

} // namespace core::config