#include "core/pch.hpp"
#include "gui/tabs/config_tab.hpp"
#include "core/config/config_manager.hpp"
#include "gui/vice_widgets.hpp"
#include "gui/vice_theme.hpp"

namespace gui {

void ConfigTab::Initialize() {
    RefreshConfigs();
    RefreshBackups("");
    LOG_INFO(GUI, "ConfigTab initialized");
}

void ConfigTab::Shutdown() {
    LOG_INFO(GUI, "ConfigTab shutdown");
}

void ConfigTab::Render() {
    ImGui::BeginChild("##config_subtabs", ImVec2(0, 30), false);
    
    const char* subTabs[] = {"Configs", "Import/Export", "Cloud Sync", "Backup"};
    for (int i = 0; i < 4; ++i) {
        bool selected = static_cast<int>(m_currentSubTab) == i;
        if (ImGui::Selectable(subTabs[i], selected, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x / 4, 0))) {
            m_currentSubTab = static_cast<SubTab>(i);
        }
        if (i < 3) ImGui::SameLine();
    }
    ImGui::EndChild();
    
    ImGui::Separator();
    
    switch (m_currentSubTab) {
        case SubTab::Configs: RenderConfigsTab(); break;
        case SubTab::ImportExport: RenderImportExportTab(); break;
        case SubTab::CloudSync: RenderCloudSyncTab(); break;
        case SubTab::Backup: RenderBackupTab(); break;
    }
}

void ConfigTab::RenderConfigsTab() {
    ImGui::BeginChild("##configs_list", ImVec2(0, 0), false);
    
    // New config button
    if (ImGui::Button("New Config")) {
        m_showNewConfigDialog = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        RefreshConfigs();
    }
    ImGui::Separator();
    
    // Config list
    if (ImGui::BeginListBox("##configs", ImVec2(-1, -1))) {
        for (size_t i = 0; i < m_configs.size(); ++i) {
            const std::string& name = m_configs[i];
            bool selected = static_cast<int>(i) == m_selectedConfig;
            
            std::string displayName = GetConfigDisplayName(name);
            if (ImGui::Selectable(displayName.c_str(), selected)) {
                m_selectedConfig = static_cast<int>(i);
            }
            
            // Context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Load")) {
                    LoadConfig(name);
                }
                if (ImGui::MenuItem("Save")) {
                    SaveConfig(name);
                }
                if (ImGui::MenuItem("Save As...")) {
                    m_renameConfigName = name;
                    m_showRenameDialog = true;
                }
                if (ImGui::MenuItem("Delete")) {
                    DeleteConfig(name);
                }
                if (ImGui::MenuItem("Create Backup")) {
                    CreateBackup(name);
                }
                ImGui::EndPopup();
            }
        }
        ImGui::EndListBox();
    }
    
    // Action buttons
    if (m_selectedConfig >= 0 && m_selectedConfig < (int)m_configs.size()) {
        ImGui::Separator();
        const std::string& name = m_configs[m_selectedConfig];
        
        if (ImGui::Button("Load")) LoadConfig(name);
        ImGui::SameLine();
        if (ImGui::Button("Save")) SaveConfig(name);
        ImGui::SameLine();
        if (ImGui::Button("Rename")) {
            m_renameConfigName = name;
            m_showRenameDialog = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete")) DeleteConfig(name);
    }
    
    // New config dialog
    if (m_showNewConfigDialog) {
        ImGui::OpenPopup("New Config");
        m_showNewConfigDialog = false;
    }
    
    if (ImGui::BeginPopupModal("New Config", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Name", &m_newConfigName);
        if (ImGui::Button("Create")) {
            if (!m_newConfigName.empty()) {
                CreateConfig(m_newConfigName);
                m_newConfigName.clear();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_newConfigName.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    // Rename dialog
    if (m_showRenameDialog) {
        ImGui::OpenPopup("Rename Config");
        m_showRenameDialog = false;
    }
    
    if (ImGui::BeginPopupModal("Rename Config", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("New Name", &m_renameConfigName);
        if (ImGui::Button("Rename")) {
            if (!m_renameConfigName.empty() && m_selectedConfig >= 0) {
                RenameConfig(m_configs[m_selectedConfig], m_renameConfigName);
                m_renameConfigName.clear();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_renameConfigName.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    ImGui::EndChild();
}

void ConfigTab::RenderImportExportTab() {
    ImGui::BeginChild("##import_export", ImVec2(0, 0), false);
    
    ImGui::Text("Import/Export Configs");
    ImGui::Separator();
    
    // Export
    ImGui::Text("Export Current Config:");
    if (ImGui::Button("Copy to Clipboard (Base64)")) {
        std::string b64 = core::config::ConfigManager::Instance().ExportToBase64Current();
        if (!b64.empty()) {
            ExportToClipboard(m_currentConfig);
            m_widgets.PushNotification("Config exported to clipboard!");
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Save to File")) {
        m_showExportDialog = true;
    }
    
    ImGui::Separator();
    
    // Import
    ImGui::Text("Import Config:");
    ImGui::InputTextMultiline("##import_base64", &m_importBase64, ImVec2(-1, 100), ImGuiInputTextFlags_AllowTabInput);
    if (ImGui::Button("Import from Base64")) {
        if (!m_importBase64.empty()) {
            std::string name = "imported_" + std::to_string(time(nullptr));
            if (core::config::ConfigManager::Instance().ImportFromBase64(name, m_importBase64)) {
                m_widgets.PushNotification("Config imported: " + name);
                RefreshConfigs();
            } else {
                m_widgets.PushNotification("Failed to import config!", gui::ViceWidgets::NotificationType::Error);
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Import from File")) {
        m_showImportDialog = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Import from Clipboard")) {
        if (core::config::ConfigManager::Instance().ImportFromClipboard("clipboard_import")) {
            m_widgets.PushNotification("Config imported from clipboard!");
            RefreshConfigs();
        } else {
            m_widgets.PushNotification("Failed to import from clipboard!", gui::ViceWidgets::NotificationType::Error);
        }
    }
    
    ImGui::EndChild();
}

void ConfigTab::RenderCloudSyncTab() {
    ImGui::BeginChild("##cloud_sync", ImVec2(0, 0), false);
    
    ImGui::Text("GitHub Gist Cloud Sync");
    ImGui::Separator();
    
    ImGui::InputText("GitHub Token", &m_cloudToken, ImGuiInputTextFlags_Password);
    ImGui::InputText("Gist ID (optional)", &m_cloudGistId);
    ImGui::Checkbox("Auto Sync", &m_config.enabled); // Would link to config
    
    ImGui::Separator();
    
    if (ImGui::Button("Sync Current to Cloud")) {
        if (!m_currentConfig.empty()) {
            if (core::config::ConfigManager::Instance().SyncToCloud(m_currentConfig, m_cloudToken, m_cloudGistId)) {
                m_widgets.PushNotification("Synced to cloud!");
            } else {
                m_widgets.PushNotification("Sync failed!", gui::ViceWidgets::NotificationType::Error);
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Sync All to Cloud")) {
        if (core::config::ConfigManager::Instance().SyncAllToCloud(m_cloudToken)) {
            m_widgets.PushNotification("All configs synced!");
        } else {
            m_widgets.PushNotification("Sync failed!", gui::ViceWidgets::NotificationType::Error);
        }
    }
    
    ImGui::Separator();
    
    if (ImGui::Button("Sync from Cloud")) {
        if (core::config::ConfigManager::Instance().SyncFromCloud(m_cloudToken, m_cloudGistId)) {
            m_widgets.PushNotification("Synced from cloud!");
            RefreshConfigs();
        } else {
            m_widgets.PushNotification("Sync failed!", gui::ViceWidgets::NotificationType::Error);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Sync All from Cloud")) {
        if (core::config::ConfigManager::Instance().SyncAllFromCloud(m_cloudToken)) {
            m_widgets.PushNotification("All configs synced from cloud!");
            RefreshConfigs();
        } else {
            m_widgets.PushNotification("Sync failed!", gui::ViceWidgets::NotificationType::Error);
        }
    }
    
    ImGui::EndChild();
}

void ConfigTab::RenderBackupTab() {
    ImGui::BeginChild("##backup_tab", ImVec2(0, 0), false);
    
    if (m_selectedConfig >= 0 && m_selectedConfig < (int)m_configs.size()) {
        const std::string& name = m_configs[m_selectedConfig];
        ImGui::Text("Backups for: %s", name.c_str());
        ImGui::Separator();
        
        if (ImGui::Button("Create Backup")) {
            CreateBackup(name);
            RefreshBackups(name);
        }
        
        if (ImGui::BeginListBox("##backups", ImVec2(-1, -1))) {
            for (size_t i = 0; i < m_backups.size(); ++i) {
                bool selected = static_cast<int>(i) == m_selectedBackup;
                std::string display = m_backups[i];
                if (ImGui::Selectable(display.c_str(), selected)) {
                    m_selectedBackup = static_cast<int>(i);
                }
            }
            ImGui::EndListBox();
        }
        
        if (m_selectedBackup >= 0 && m_selectedBackup < (int)m_backups.size()) {
            ImGui::Separator();
            if (ImGui::Button("Restore")) {
                if (RestoreBackup(name, m_backups[m_selectedBackup])) {
                    m_widgets.PushNotification("Backup restored!");
                } else {
                    m_widgets.PushNotification("Restore failed!", gui::ViceWidgets::NotificationType::Error);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                DeleteBackup(name, m_backups[m_selectedBackup]);
                RefreshBackups(name);
            }
        }
    } else {
        ImGui::Text("Select a config to manage backups.");
    }
    
    ImGui::EndChild();
}

// Operations
void ConfigTab::RefreshConfigs() {
    m_configs = core::config::ConfigManager::Instance().ListConfigs();
    if (m_selectedConfig >= (int)m_configs.size()) {
        m_selectedConfig = -1;
    }
}

void ConfigTab::RefreshBackups(const std::string& configName) {
    m_backups = core::config::ConfigManager::Instance().ListBackups(configName);
    m_selectedBackup = -1;
}

bool ConfigTab::CreateConfig(const std::string& name) {
    if (core::config::ConfigManager::Instance().Save(name)) {
        RefreshConfigs();
        m_widgets.PushNotification("Config created: " + name);
        return true;
    }
    m_widgets.PushNotification("Failed to create config!", gui::ViceWidgets::NotificationType::Error);
    return false;
}

bool ConfigTab::DeleteConfig(const std::string& name) {
    if (core::config::ConfigManager::Instance().Delete(name)) {
        RefreshConfigs();
        m_widgets.PushNotification("Config deleted: " + name);
        return true;
    }
    m_widgets.PushNotification("Failed to delete config!", gui::ViceWidgets::NotificationType::Error);
    return false;
}

bool ConfigTab::RenameConfig(const std::string& oldName, const std::string& newName) {
    if (core::config::ConfigManager::Instance().Rename(oldName, newName)) {
        RefreshConfigs();
        m_widgets.PushNotification("Config renamed: " + oldName + " -> " + newName);
        return true;
    }
    m_widgets.PushNotification("Failed to rename config!", gui::ViceWidgets::NotificationType::Error);
    return false;
}

bool ConfigTab::LoadConfig(const std::string& name) {
    if (core::config::ConfigManager::Instance().Load(name)) {
        m_widgets.PushNotification("Config loaded: " + name);
        return true;
    }
    m_widgets.PushNotification("Failed to load config!", gui::ViceWidgets::NotificationType::Error);
    return false;
}

bool ConfigTab::SaveConfig(const std::string& name) {
    if (core::config::ConfigManager::Instance().Save(name)) {
        m_widgets.PushNotification("Config saved: " + name);
        return true;
    }
    m_widgets.PushNotification("Failed to save config!", gui::ViceWidgets::NotificationType::Error);
    return false;
}

bool ConfigTab::SaveConfigAs(const std::string& name) {
    return SaveConfig(name);
}

// Import/Export
bool ConfigTab::ExportToBase64(const std::string& name, std::string& outBase64) {
    outBase64 = core::config::ConfigManager::Instance().ExportToBase64(name);
    return !outBase64.empty();
}

bool ConfigTab::ImportFromBase64(const std::string& name, const std::string& base64) {
    return core::config::ConfigManager::Instance().ImportFromBase64(name, base64);
}

bool ConfigTab::ExportToFile(const std::string& name, const std::string& path) {
    return core::config::ConfigManager::Instance().ExportToFile(name, path);
}

bool ConfigTab::ImportFromFile(const std::string& name, const std::string& path) {
    return core::config::ConfigManager::Instance().ImportFromFile(name, path);
}

bool ConfigTab::ExportToClipboard(const std::string& name) {
    return core::config::ConfigManager::Instance().ExportToClipboard(name);
}

bool ConfigTab::ImportFromClipboard(const std::string& name) {
    return core::config::ConfigManager::Instance().ImportFromClipboard(name);
}

// Cloud Sync
bool ConfigTab::SyncToCloud(const std::string& name) {
    return core::config::ConfigManager::Instance().SyncToCloud(name, m_cloudToken, m_cloudGistId);
}

bool ConfigTab::SyncAllToCloud() {
    return core::config::ConfigManager::Instance().SyncAllToCloud(m_cloudToken);
}

bool ConfigTab::SyncFromCloud() {
    return core::config::ConfigManager::Instance().SyncFromCloud(m_cloudToken, m_cloudGistId);
}

bool ConfigTab::SyncAllFromCloud() {
    return core::config::ConfigManager::Instance().SyncAllFromCloud(m_cloudToken);
}

// Backup
void ConfigTab::CreateBackup(const std::string& name) {
    core::config::ConfigManager::Instance().CreateBackup(name);
    RefreshBackups(name);
    m_widgets.PushNotification("Backup created!");
}

bool ConfigTab::RestoreBackup(const std::string& name, const std::string& backupName) {
    if (core::config::ConfigManager::Instance().RestoreBackup(name, backupName)) {
        RefreshBackups(name);
        return true;
    }
    return false;
}

bool ConfigTab::DeleteBackup(const std::string& name, const std::string& backupName) {
    if (core::config::ConfigManager::Instance().DeleteBackup(name, backupName)) {
        RefreshBackups(name);
        return true;
    }
    return false;
}

// Helpers
std::string ConfigTab::GetConfigDisplayName(const std::string& name) const {
    auto meta = core::config::ConfigManager::Instance().GetMetadata(name);
    std::string display = name;
    if (!meta.modified.empty()) {
        display += " [" + meta.modified + "]";
    }
    if (meta.isAutoSave) display += " (Auto)";
    if (meta.isCloudSynced) display += " (Cloud)";
    return display;
}

std::string ConfigTab::GetConfigTimeString(const std::string& name) const {
    auto meta = core::config::ConfigManager::Instance().GetMetadata(name);
    return meta.modified;
}

std::string ConfigTab::GetBackupTimeString(const std::string& name, const std::string& backupName) const {
    return backupName;
}

} // namespace gui