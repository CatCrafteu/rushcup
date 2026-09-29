#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace Launcher {

struct Config {
    std::string steamPath = "";
    std::string cs2AppId = "730";
    std::string dllName = "ViceCity.dll";
    std::string windowTitle = "ViceCity Launcher";
    bool autoClose = true;
    int injectDelay = 3000; // ms
    
    static Config Load(const std::string& path = "launcher_config.json") {
        Config cfg;
        
        // Try config file first
        if (std::filesystem::exists(path)) {
            try {
                std::ifstream file(path);
                nlohmann::json j;
                file >> j;
                
                cfg.steamPath = j.value("steamPath", "");
                cfg.cs2AppId = j.value("cs2AppId", "730");
                cfg.dllName = j.value("dllName", "ViceCity.dll");
                cfg.windowTitle = j.value("windowTitle", "ViceCity Launcher");
                cfg.autoClose = j.value("autoClose", true);
                cfg.injectDelay = j.value("injectDelay", 3000);
            } catch (...) {}
        }
        
        // Auto-detect Steam if not configured
        if (cfg.steamPath.empty()) {
            cfg.steamPath = AutoDetectSteam();
        }
        
        return cfg;
    }
    
    void Save(const std::string& path = "launcher_config.json") const {
        nlohmann::json j;
        j["steamPath"] = steamPath;
        j["cs2AppId"] = cs2AppId;
        j["dllName"] = dllName;
        j["windowTitle"] = windowTitle;
        j["autoClose"] = autoClose;
        j["injectDelay"] = injectDelay;
        
        std::ofstream file(path);
        file << j.dump(4);
    }
    
    static std::string AutoDetectSteam() {
        // Registry
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char path[MAX_PATH];
            DWORD size = sizeof(path);
            if (RegQueryValueExA(hKey, "SteamPath", nullptr, nullptr, (LPBYTE)path, &size) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return std::string(path);
            }
            RegCloseKey(hKey);
        }
        
        // Common paths
        std::vector<std::string> common = {
            "C:\\Program Files (x86)\\Steam",
            "C:\\Program Files\\Steam",
            "D:\\Steam",
            "E:\\Steam"
        };
        
        for (const auto& p : common) {
            if (std::filesystem::exists(p + "\\steam.exe")) return p;
        }
        
        return "";
    }
};

} // namespace Launcher