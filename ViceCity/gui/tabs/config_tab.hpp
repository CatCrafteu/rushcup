#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <filesystem>
#include "../../core/config/config_manager.hpp"
#include "../vice_widgets.hpp"
#include "../vice_theme.hpp"

namespace gui {

class ConfigTab {
public:
    static ConfigTab& Instance() {
        static ConfigTab instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
private:
    ConfigTab() = default;
    
    enum class SubTab { Configs, ImportExport, CloudSync, Backup };
    SubTab m_currentSubTab = SubTab::Configs;
    
    // State
    std::string m_newConfigName;
    std::string m_importBase64;
    std::string m_cloudToken;
    std::string m_cloudGistId;
    std::string m_exportPath;
    int m_selectedConfig = -1;
    int m_selectedBackup = -1;
    bool m_showNewConfigDialog = false;
    bool m_showImportDialog = false;
    bool m_showExportDialog = false;
    bool m_showCloudSyncDialog = false;
    bool m_showRenameDialog = false;
    std::string m_renameConfigName;
    
    // Config list
    std::vector<std::string> m_configs;
    std::vector<std::string> m_backups;
    std::filesystem::file_time_type m_lastRefresh;
    
    void RenderConfigsTab();
    void RenderImportExportTab();
    void RenderCloudSyncTab();
    void RenderBackupTab();
    
    // Config operations
    void RefreshConfigs();
    void RefreshBackups(const std::string& configName);
    bool CreateConfig(const std::string& name);
    bool DeleteConfig(const std::string& name);
    bool RenameConfig(const std::string& oldName, const std::string& newName);
    bool LoadConfig(const std::string& name);
    bool SaveConfig(const std::string& name);
    bool SaveConfigAs(const std::string& name);
    
    // Import/Export
    bool ExportToBase64(const std::string& name, std::string& outBase64);
    bool ImportFromBase64(const std::string& name, const std::string& base64);
    bool ExportToFile(const std::string& name, const std::string& path);
    bool ImportFromFile(const std::string& name, const std::string& path);
    bool ExportToClipboard(const std::string& name);
    bool ImportFromClipboard(const std::string& name);
    
    // Cloud sync
    bool SyncToCloud(const std::string& name);
    bool SyncAllToCloud();
    bool SyncFromCloud();
    bool SyncAllFromCloud();
    
    // Backup
    void CreateBackup(const std::string& name);
    bool RestoreBackup(const std::string& name, const std::string& backupName);
    bool DeleteBackup(const std::string& name, const std::string& backupName);
    
    // Helpers
    std::string GetConfigDisplayName(const std::string& name) const;
    std::string GetConfigTimeString(const std::string& name) const;
    std::string GetBackupTimeString(const std::string& name, const std::string& backupName) const;
    
    // Theme access
    gui::ViceTheme::Colors& m_colors = gui::ViceTheme::Instance().GetColors();
    gui::ViceWidgets& m_widgets = gui::ViceWidgets::Instance();
};

} // namespace gui