#include "core/pch.hpp"
#include "core/config/config_manager.hpp"
#include <shlobj.h>

namespace core::config {

bool ConfigManager::Initialize(const std::string& configDir) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) return true;
    
    // Get app data path
    char appData[MAX_PATH];
    if (SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, appData) != S_OK) {
        return false;
    }
    
    m_configDir = std::filesystem::path(appData) / configDir;
    m_backupDir = m_configDir / "backups";
    
    EnsureDirectories();
    LoadSchema();
    
    m_initialized = true;
    LOG_INFO("Config", "ConfigManager initialized at: {}", m_configDir.string());
    return true;
}

void ConfigManager::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_autoSave && m_modified) {
        Save(m_currentConfig);
    }
    
    m_initialized = false;
    LOG_INFO("Config", "ConfigManager shutdown");
}

void ConfigManager::EnsureDirectories() {
    std::filesystem::create_directories(m_configDir);
    std::filesystem::create_directories(m_backupDir);
}

void ConfigManager::LoadSchema() {
    // Use default schema
    m_schema = DEFAULT_SCHEMA;
    m_schemaInfo.version = DEFAULT_SCHEMA["version"];
    m_schemaInfo.name = DEFAULT_SCHEMA["name"];
    m_schemaInfo.description = DEFAULT_SCHEMA["description"];
    m_schemaInfo.schema = DEFAULT_SCHEMA["schema"];
}

void ConfigManager::SaveSchema() {
    std::filesystem::path schemaPath = m_configDir / "schema.json";
    std::ofstream file(schemaPath);
    if (file.is_open()) {
        file << m_schema.dump(4);
    }
}

bool ConfigManager::Load(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::filesystem::path configPath = GetConfigPath(name);
    if (!std::filesystem::exists(configPath)) {
        LOG_WARN("Config", "Config not found: {}", name);
        return false;
    }
    
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) return false;
        
        json loadedConfig;
        file >> loadedConfig;
        
        // Validate and merge with defaults
        if (ValidateConfig(loadedConfig)) {
            MergeDefaults(loadedConfig, m_schema["schema"]);
            m_config = std::move(loadedConfig);
            m_currentConfig = name;
            m_modified = false;
            
            // Update last write time
            m_lastWriteTime = std::filesystem::last_write_time(configPath);
            
            TriggerLoadCallbacks(name);
            LOG_INFO("Config", "Loaded config: {}", name);
            return true;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Failed to load config {}: {}", name, e.what());
    }
    
    return false;
}

bool ConfigManager::Save(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (name.empty()) return false;
    
    std::filesystem::path configPath = GetConfigPath(name);
    
    try {
        // Create metadata
        json output = m_config;
        output["_meta"] = {
            {"name", name},
            {"version", m_schemaInfo.version},
            {"saved", GetCurrentTimestamp()},
            {"vicecity_version", VICECITY_VERSION}
        };
        
        std::ofstream file(configPath);
        if (!file.is_open()) return false;
        
        file << output.dump(4);
        file.close();
        
        m_currentConfig = name;
        m_modified = false;
        m_lastWriteTime = std::filesystem::last_write_time(configPath);
        
        TriggerSaveCallbacks(name);
        LOG_INFO("Config", "Saved config: {}", name);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Failed to save config {}: {}", name, e.what());
    }
    
    return false;
}

bool ConfigManager::SaveAs(const std::string& name) {
    return Save(name);
}

bool ConfigManager::Delete(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::filesystem::path configPath = GetConfigPath(name);
    if (!std::filesystem::exists(configPath)) return false;
    
    try {
        std::filesystem::remove(configPath);
        
        // Also remove backups
        std::filesystem::path backupPath = m_backupDir / name;
        if (std::filesystem::exists(backupPath)) {
            std::filesystem::remove_all(backupPath);
        }
        
        if (m_currentConfig == name) {
            m_currentConfig.clear();
            m_config.clear();
        }
        
        LOG_INFO("Config", "Deleted config: {}", name);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Failed to delete config {}: {}", name, e.what());
    }
    
    return false;
}

bool ConfigManager::Rename(const std::string& oldName, const std::string& newName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::filesystem::path oldPath = GetConfigPath(oldName);
    std::filesystem::path newPath = GetConfigPath(newName);
    
    if (!std::filesystem::exists(oldPath) || std::filesystem::exists(newPath)) {
        return false;
    }
    
    try {
        std::filesystem::rename(oldPath, newPath);
        
        // Rename backups
        std::filesystem::path oldBackupDir = m_backupDir / oldName;
        std::filesystem::path newBackupDir = m_backupDir / newName;
        if (std::filesystem::exists(oldBackupDir)) {
            std::filesystem::rename(oldBackupDir, newBackupDir);
        }
        
        if (m_currentConfig == oldName) {
            m_currentConfig = newName;
        }
        
        LOG_INFO("Config", "Renamed config: {} -> {}", oldName, newName);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Failed to rename config: {}", e.what());
    }
    
    return false;
}

std::vector<std::string> ConfigManager::ListConfigs() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> configs;
    
    if (!std::filesystem::exists(m_configDir)) return configs;
    
    for (const auto& entry : std::filesystem::directory_iterator(m_configDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            std::string name = entry.path().stem().string();
            if (name != "schema") {
                configs.push_back(name);
            }
        }
    }
    
    std::sort(configs.begin(), configs.end());
    return configs;
}

std::vector<std::string> ConfigManager::ListConfigsForMap(const std::string& map) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> configs;
    
    auto allConfigs = ListConfigs();
    for (const auto& name : allConfigs) {
        auto meta = GetMetadata(name);
        if (meta.mapName == map || meta.mapName.empty()) {
            configs.push_back(name);
        }
    }
    
    return configs;
}

std::string ConfigManager::ExportToBase64(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    json config = (name.empty() ? m_config : GetConfig(name));
    std::string jsonStr = config.dump();
    return Base64Encode(jsonStr);
}

std::string ConfigManager::ExportToBase64Current() const {
    return ExportToBase64(m_currentConfig);
}

bool ConfigManager::ImportFromBase64(const std::string& name, const std::string& base64) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try {
        std::string jsonStr = Base64Decode(base64);
        json config = json::parse(jsonStr);
        
        if (ValidateConfig(config)) {
            ApplyDefaults(m_schema["schema"]);
            
            std::filesystem::path configPath = GetConfigPath(name);
            std::ofstream file(configPath);
            if (!file.is_open()) return false;
            
            file << config.dump(4);
            return true;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Failed to import from base64: {}", e.what());
    }
    
    return false;
}

bool ConfigManager::ImportFromBase64Current(const std::string& base64) {
    return ImportFromBase64(m_currentConfig, base64);
}

bool ConfigManager::ExportToFile(const std::string& name, const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    json config = (name.empty() ? m_config : GetConfig(name));
    
    try {
        std::ofstream file(path);
        if (!file.is_open()) return false;
        
        file << config.dump(4);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Failed to export to file: {}", e.what());
    }
    
    return false;
}

bool ConfigManager::ImportFromFile(const std::string& name, const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        
        json config;
        file >> config;
        
        if (ValidateConfig(config)) {
            ApplyDefaults(m_schema["schema"]);
            
            std::filesystem::path configPath = GetConfigPath(name);
            std::ofstream outFile(configPath);
            if (!outFile.is_open()) return false;
            
            outFile << config.dump(4);
            return true;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Config", "Failed to import from file: {}", e.what());
    }
    
    return false;
}

bool ConfigManager::ExportToClipboard(const std::string& name) const {
    std::string base64 = ExportToBase64(name);
    if (base64.empty()) return false;
    
    if (!OpenClipboard(nullptr)) return false;
    EmptyClipboard();
    
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, base64.size() + 1);
    if (!hGlobal) {
        CloseClipboard();
        return false;
    }
    
    char* pGlobal = static_cast<char*>(GlobalLock(hGlobal));
    if (pGlobal) {
        strcpy_s(pGlobal, base64.size() + 1, base64.c_str());
        GlobalUnlock(hGlobal);
        SetClipboardData(CF_TEXT, hGlobal);
        CloseClipboard();
        return true;
    }
    
    GlobalFree(hGlobal);
    CloseClipboard();
    return false;
}

bool ConfigManager::ImportFromClipboard(const std::string& name) {
    if (!OpenClipboard(nullptr)) return false;
    
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (!hData) {
        CloseClipboard();
        return false;
    }
    
    char* pData = static_cast<char*>(GlobalLock(hData));
    if (!pData) {
        GlobalUnlock(hData);
        CloseClipboard();
        return false;
    }
    
    std::string base64(pData);
    GlobalUnlock(hData);
    CloseClipboard();
    
    return ImportFromBase64(name, base64);
}

bool ConfigManager::SyncToCloud(const std::string& name, const std::string& gistToken, const std::string& gistId) {
    // TODO: Implement GitHub Gist API integration
    LOG_WARN("Config", "Cloud sync not yet implemented");
    return false;
}

bool ConfigManager::SyncFromCloud(const std::string& gistToken, const std::string& gistId) {
    LOG_WARN("Config", "Cloud sync not yet implemented");
    return false;
}

bool ConfigManager::SyncAllToCloud(const std::string& gistToken) {
    LOG_WARN("Config", "Cloud sync not yet implemented");
    return false;
}

bool ConfigManager::SyncAllFromCloud(const std::string& gistToken) {
    LOG_WARN("Config", "Cloud sync not yet implemented");
    return false;
}

json& ConfigManager::GetConfig(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    static json empty;
    
    if (name == m_currentConfig) return m_config;
    
    // Load temporarily
    std::filesystem::path configPath = GetConfigPath(name);
    if (!std::filesystem::exists(configPath)) return empty;
    
    try {
        std::ifstream file(configPath);
        json config;
        file >> config;
        // Store temporarily (not ideal but works for read-only access)
        return config;
    } catch (...) {
        return empty;
    }
}

const json& ConfigManager::GetConfig(const std::string& name) const {
    return const_cast<ConfigManager*>(this)->GetConfig(name);
}

bool ConfigManager::HasKey(const std::string& key) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_config.contains(key);
}

bool ConfigManager::HasKey(const std::string& category, const std::string& key) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_config.contains(category) && m_config[category].contains(key);
}

void ConfigManager::RemoveKey(const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.erase(key);
    m_modified = true;
}

void ConfigManager::RemoveKey(const std::string& category, const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_config.contains(category)) {
        m_config[category].erase(key);
        m_modified = true;
    }
}

void ConfigManager::ClearCategory(const std::string& category) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.erase(category);
    m_modified = true;
}

bool ConfigManager::ValidateSchema(const json& schema) {
    // TODO: Implement JSON schema validation
    return true;
}

void ConfigManager::ApplyDefaults(const json& defaults) {
    MergeDefaults(m_config, defaults);
    m_modified = true;
}

void ConfigManager::MergeDefaults(json& config, const json& defaults) {
    for (auto& [key, value] : defaults.items()) {
        if (!config.contains(key)) {
            config[key] = value;
        } else if (value.is_object() && config[key].is_object()) {
            MergeDefaults(config[key], value);
        }
    }
}

void ConfigManager::EnableAutoSave(bool enable, int intervalMs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_autoSave = enable;
    m_autoSaveInterval = intervalMs;
}

void ConfigManager::TriggerAutoSave() {
    if (m_autoSave && m_modified && !m_currentConfig.empty()) {
        Save(m_currentConfig);
    }
}

void ConfigManager::SetMapConfig(const std::string& map, const std::string& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config["config"]["per_map_configs"][map] = config;
    m_modified = true;
}

std::optional<std::string> ConfigManager::GetMapConfig(const std::string& map) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_config.contains("config") && m_config["config"].contains("per_map_configs")) {
        auto& mapConfigs = m_config["config"]["per_map_configs"];
        if (mapConfigs.contains(map)) {
            return mapConfigs[map].get<std::string>();
        }
    }
    return std::nullopt;
}

void ConfigManager::EnableHotReload(bool enable) {
    m_hotReload = enable;
    if (enable && !m_currentConfig.empty()) {
        m_lastWriteTime = std::filesystem::last_write_time(GetConfigPath(m_currentConfig));
    }
}

void ConfigManager::CheckHotReload() {
    if (!m_hotReload || m_currentConfig.empty()) return;
    
    std::filesystem::path configPath = GetConfigPath(m_currentConfig);
    if (!std::filesystem::exists(configPath)) return;
    
    auto currentWriteTime = std::filesystem::last_write_time(configPath);
    if (currentWriteTime > m_lastWriteTime) {
        m_lastWriteTime = currentWriteTime;
        Load(m_currentConfig);
        LOG_INFO("Config", "Hot reloaded config: {}", m_currentConfig);
    }
}







ConfigManager::ConfigMetadata ConfigManager::GetMetadata(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    ConfigMetadata meta;
    
    std::filesystem::path configPath = GetConfigPath(name);
    if (!std::filesystem::exists(configPath)) return meta;
    
    try {
        std::ifstream file(configPath);
        json config;
        file >> config;
        
        if (config.contains("_meta")) {
            auto& m = config["_meta"];
            meta.name = m.value("name", name);
            meta.version = m.value("version", 1);
            meta.created = m.value("created", "");
            meta.modified = m.value("saved", "");
            
            if (config.contains("config") && config["config"].contains("per_map_configs")) {
                auto& mapConfigs = config["config"]["per_map_configs"];
                for (auto& [map, cfg] : mapConfigs.items()) {
                    if (!cfg.is_null() && !cfg.get<std::string>().empty()) {
                        meta.mapName = map;
                        break;
                    }
                }
            }
        }
    } catch (...) {}
    
    return meta;
}

void ConfigManager::SetMetadata(const std::string& name, const ConfigMetadata& meta) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::filesystem::path configPath = GetConfigPath(name);
    if (!std::filesystem::exists(configPath)) return;
    
    try {
        std::ifstream file(configPath);
        json config;
        file >> config;
        
        config["_meta"] = {
            {"name", meta.name},
            {"version", meta.version},
            {"created", meta.created.empty() ? GetCurrentTimestamp() : meta.created},
            {"saved", GetCurrentTimestamp()},
            {"vicecity_version", VICECITY_VERSION}
        };
        
        std::ofstream outFile(configPath);
        outFile << config.dump(4);
    } catch (...) {}
}

void ConfigManager::CreateBackup(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::filesystem::path configPath = GetConfigPath(name);
    if (!std::filesystem::exists(configPath)) return;
    
    std::filesystem::path backupDir = m_backupDir / name;
    std::filesystem::create_directories(backupDir);
    
    std::string timestamp = GetCurrentTimestamp();
    std::replace(timestamp.begin(), timestamp.end(), ':', '-');
    std::filesystem::path backupPath = backupDir / (name + "_" + timestamp + ".json");
    
    try {
        std::filesystem::copy_file(configPath, backupPath);
        
        // Clean old backups (keep last 10)
        std::vector<std::filesystem::path> backups;
        for (const auto& entry : std::filesystem::directory_iterator(backupDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                backups.push_back(entry.path());
            }
        }
        
        std::sort(backups.begin(), backups.end(), 
            [](const auto& a, const auto& b) {
                return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
            });
        
        for (size_t i = 10; i < backups.size(); ++i) {
            std::filesystem::remove(backups[i]);
        }
    } catch (...) {}
}

std::vector<std::string> ConfigManager::ListBackups(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> backups;
    
    std::filesystem::path backupDir = m_backupDir / name;
    if (!std::filesystem::exists(backupDir)) return backups;
    
    for (const auto& entry : std::filesystem::directory_iterator(backupDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            backups.push_back(entry.path().stem().string());
        }
    }
    
    std::sort(backups.begin(), backups.end(), std::greater<>());
    return backups;
}

bool ConfigManager::RestoreBackup(const std::string& name, const std::string& backupName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::filesystem::path backupPath = m_backupDir / name / (backupName + ".json");
    std::filesystem::path configPath = GetConfigPath(name);
    
    if (!std::filesystem::exists(backupPath)) return false;
    
    try {
        std::filesystem::copy_file(backupPath, configPath, std::filesystem::copy_options::overwrite_existing);
        return Load(name);
    } catch (...) {}
    
    return false;
}

std::string ConfigManager::GetConfigPath(const std::string& name) const {
    return (m_configDir / (name + ".json")).string();
}

std::string ConfigManager::GetCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string ConfigManager::Base64Encode(const std::string& input) const {
    static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string output;
    output.reserve((input.size() + 2) / 3 * 4);
    
    for (size_t i = 0; i < input.size(); i += 3) {
        uint32_t octet = 0;
        int val = 0;
        
        for (int j = 0; j < 3; ++j) {
            octet <<= 8;
            if (i + j < input.size()) {
                octet |= static_cast<uint8_t>(input[i + j]);
                val++;
            }
        }
        
        for (int j = 0; j < 4; ++j) {
            if (j < val + 1) {
                output.push_back(encoding[(octet >> (18 - j * 6)) & 0x3F]);
            } else {
                output.push_back('=');
            }
        }
    }
    
    return output;
}

std::string ConfigManager::Base64Decode(const std::string& input) const {
    static const std::string encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string output;
    output.reserve(input.size() * 3 / 4);
    
    for (size_t i = 0; i < input.size(); i += 4) {
        uint32_t octet = 0;
        int val = 0;
        
        for (int j = 0; j < 4; ++j) {
            if (i + j < input.size() && input[i + j] != '=') {
                auto pos = encoding.find(input[i + j]);
                if (pos != std::string::npos) {
                    octet = (octet << 6) | pos;
                    val++;
                }
            }
        }
        
        for (int j = val - 1; j >= 0; --j) {
            output.push_back(static_cast<char>((octet >> (j * 8)) & 0xFF));
        }
    }
    
    return output;
}

bool ConfigManager::ValidateConfig(const json& config) const {
    // Basic validation
    if (!config.is_object()) return false;
    return true;
}

void ConfigManager::TriggerLoadCallbacks(const std::string& name) {
    for (auto& cb : m_onLoadCallbacks) {
        try {
            cb(name, m_config);
        } catch (...) {}
    }
}

void ConfigManager::TriggerSaveCallbacks(const std::string& name) {
    for (auto& cb : m_onSaveCallbacks) {
        try {
            cb(name, m_config);
        } catch (...) {}
    }
}

void ConfigManager::TriggerChangeCallbacks(const std::string& name) {
    for (auto& cb : m_onChangeCallbacks) {
        try {
            cb(name, m_config);
        } catch (...) {}
    }
}

} // namespace core::config
