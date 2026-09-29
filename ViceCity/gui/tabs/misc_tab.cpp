#include "core/pch.hpp"
#include "gui/tabs/misc_tab.hpp"
#include "features/misc/misc.hpp"
#include "features/misc/skinchanger.hpp"
#include "features/misc/inventory_ui.hpp"
#include "features/lua/lua_manager.hpp"
#include "gui/vice_widgets.hpp"
#include "gui/vice_theme.hpp"

namespace gui {

void MiscTab::Initialize() {
    LOG_INFO(GUI, "MiscTab initialized");
}

void MiscTab::Shutdown() {
    LOG_INFO(GUI, "MiscTab shutdown");
}

void MiscTab::Render() {
    ImGui::BeginChild("##misc_subtabs", ImVec2(0, 30), false);
    
    const char* subTabs[] = {"Movement", "Logs", "Skinchanger", "Inventory UI", "Lua"};
    for (int i = 0; i < 5; ++i) {
        bool selected = static_cast<int>(m_currentSubTab) == i;
        if (ImGui::Selectable(subTabs[i], selected, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x / 5, 0))) {
            m_currentSubTab = static_cast<SubTab>(i);
        }
        if (i < 4) ImGui::SameLine();
    }
    ImGui::EndChild();
    
    ImGui::Separator();
    
    switch (m_currentSubTab) {
        case SubTab::Movement: RenderMovementTab(); break;
        case SubTab::Logs: RenderLogsTab(); break;
        case SubTab::Skinchanger: RenderSkinchangerTab(); break;
        case SubTab::InventoryUI: RenderInventoryUITab(); break;
        case SubTab::Lua: RenderLuaTab(); break;
    }
}

void MiscTab::RenderMovementTab() {
    ImGui::BeginChild("##movement_tab", ImVec2(0, 0), false);
    
    RenderAutoPeekSettings();
    ImGui::Separator();
    
    RenderThirdpersonSettings();
    ImGui::Separator();
    
    RenderFOVOverrideSettings();
    ImGui::Separator();
    
    RenderViewmodelChangerSettings();
    ImGui::Separator();
    
    RenderAspectRatioSettings();
    ImGui::Separator();
    
    RenderRecoilCrosshairSettings();
    ImGui::Separator();
    
    RenderPenetrationCrosshairSettings();
    
    ImGui::EndChild();
}

void MiscTab::RenderLogsTab() {
    ImGui::BeginChild("##logs_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Hit Logs", &m_logsConfig.hitLogs);
    ImGui::Checkbox("Damage Logs", &m_logsConfig.damageLogs);
    ImGui::Checkbox("Purchase Logs", &m_logsConfig.purchaseLogs);
    ImGui::Checkbox("Console Logs", &m_logsConfig.consoleLogs);
    ImGui::Separator();
    
    ImGui::Checkbox("Filter Local", &m_logsConfig.filterLocal);
    ImGui::Checkbox("Filter Teammates", &m_logsConfig.filterTeammates);
    ImGui::Checkbox("Filter Enemies", &m_logsConfig.filterEnemies);
    
    ImGui::EndChild();
}

void MiscTab::RenderSkinchangerTab() {
    ImGui::BeginChild("##skinchanger_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Enabled", &m_skinchangerConfig.enabled);
    ImGui::Checkbox("Auto Apply", &m_skinchangerConfig.autoApply);
    ImGui::Checkbox("Show in Inventory", &m_skinchangerConfig.showInInventory);
    ImGui::Separator();
    
    RenderWeaponSkinsList();
    ImGui::Separator();
    
    RenderKnifeGloveSettings();
    
    ImGui::EndChild();
}

void MiscTab::RenderInventoryUITab() {
    ImGui::BeginChild("##inventory_ui_tab", ImVec2(0, 0), false);
    
    ImGui::Checkbox("Case Opening", &m_inventoryUIConfig.showCaseOpening);
    ImGui::Checkbox("Sticker Tool", &m_inventoryUIConfig.showStickerTool);
    ImGui::Checkbox("Inspect Panel", &m_inventoryUIConfig.showInspectPanel);
    ImGui::Checkbox("Pattern Seed Browser", &m_inventoryUIConfig.showPatternSeedBrowser);
    ImGui::Separator();
    
    RenderCaseOpeningSettings();
    ImGui::Separator();
    
    RenderStickerToolSettings();
    ImGui::Separator();
    
    RenderInspectPanelSettings();
    ImGui::Separator();
    
    RenderPatternSeedBrowserSettings();
    
    ImGui::EndChild();
}

void MiscTab::RenderLuaTab() {
    ImGui::BeginChild("##lua_tab", ImVec2(0, 0), false);
    
    ImGui::Text("Lua scripting is available in the dedicated Lua tab.");
    ImGui::Text("This tab shows basic status.");
    ImGui::Separator();
    
    auto& lua = features::lua::LuaManager::Instance();
    ImGui::Text("Loaded Scripts: %zu", lua.GetLoadedScripts().size());
    ImGui::Text("Available Scripts: %zu", lua.GetAvailableScripts().size());
    
    if (ImGui::Button("Refresh Scripts")) {
        // lua.RefreshScripts();
    }
    
    ImGui::EndChild();
}

// Movement Helpers
void MiscTab::RenderAutoPeekSettings() {
    ImGui::Text("Auto Peek");
    ImGui::Checkbox("Enabled", &m_movementConfig.autoPeek.enabled);
    // Keybind widget
    ImGui::Checkbox("Render", &m_movementConfig.autoPeek.render);
}

void MiscTab::RenderThirdpersonSettings() {
    ImGui::Text("Thirdperson");
    ImGui::Checkbox("Enabled", &m_movementConfig.thirdperson.enabled);
    // Keybind widget
    ImGui::DragFloat("Distance", &m_movementConfig.thirdperson.distance, 1.0f, 50.0f, 500.0f);
    ImGui::Checkbox("Collision", &m_movementConfig.thirdperson.collision);
}

void MiscTab::RenderFOVOverrideSettings() {
    ImGui::Text("FOV Override");
    ImGui::Checkbox("Enabled", &m_movementConfig.fovOverride.enabled);
    ImGui::DragFloat("Value", &m_movementConfig.fovOverride.value, 1.0f, 60.0f, 140.0f);
}

void MiscTab::RenderViewmodelChangerSettings() {
    ImGui::Text("Viewmodel Changer");
    ImGui::Checkbox("Enabled", &m_movementConfig.viewmodelChanger.enabled);
    ImGui::DragFloat("X", &m_movementConfig.viewmodelChanger.x, 0.1f, -10.0f, 10.0f);
    ImGui::DragFloat("Y", &m_movementConfig.viewmodelChanger.y, 0.1f, -10.0f, 10.0f);
    ImGui::DragFloat("Z", &m_movementConfig.viewmodelChanger.z, 0.1f, -10.0f, 10.0f);
    ImGui::DragFloat("FOV", &m_movementConfig.viewmodelChanger.fov, 1.0f, 30.0f, 120.0f);
}

void MiscTab::RenderAspectRatioSettings() {
    ImGui::Text("Aspect Ratio");
    ImGui::Checkbox("Enabled", &m_movementConfig.aspectRatio.enabled);
    ImGui::DragFloat("Value", &m_movementConfig.aspectRatio.value, 0.01f, 1.0f, 2.5f);
}

void MiscTab::RenderRecoilCrosshairSettings() {
    ImGui::Text("Recoil Crosshair");
    ImGui::Checkbox("Enabled", &m_movementConfig.recoilCrosshair.enabled);
    // Color picker
}

void MiscTab::RenderPenetrationCrosshairSettings() {
    ImGui::Text("Penetration Crosshair");
    ImGui::Checkbox("Enabled", &m_movementConfig.penetrationCrosshair.enabled);
    // Color picker
}

// Skinchanger Helpers
void MiscTab::RenderWeaponSkinsList() {
    ImGui::Text("Weapon Skins");
    ImGui::Text("Configure weapon skins in the Inventory UI tab.");
    ImGui::Text("Use the Skinchanger tab for basic enable/disable.");
}

void MiscTab::RenderKnifeGloveSettings() {
    ImGui::Text("Knife / Glove Models");
    ImGui::Text("Configure in Inventory UI > Skinchanger section.");
}

// Inventory UI Helpers
void MiscTab::RenderCaseOpeningSettings() {
    ImGui::Text("Case Opening");
    ImGui::Text("Configure cases/keys in Inventory UI > Case Opening section.");
}

void MiscTab::RenderStickerToolSettings() {
    ImGui::Text("Sticker Tool");
    ImGui::Text("Configure in Inventory UI > Sticker Tool section.");
}

void MiscTab::RenderInspectPanelSettings() {
    ImGui::Text("Inspect Panel");
    ImGui::Text("Press F in-game to inspect weapons.");
}

void MiscTab::RenderPatternSeedBrowserSettings() {
    ImGui::Text("Pattern Seed Browser");
    ImGui::Text("Browse seeds 0-1000 in Inventory UI > Pattern Seed section.");
}

} // namespace gui