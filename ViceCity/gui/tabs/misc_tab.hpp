#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include "../../features/misc/misc.hpp"
#include "../../features/misc/misc.hpp"
#include "../../features/misc/misc.hpp"
#include "../../features/lua/lua_manager.hpp"
#include "../../core/config/config_manager.hpp"
#include "../vice_widgets.hpp"
#include "../vice_theme.hpp"

namespace gui {

class MiscTab {
public:
    static MiscTab& Instance() {
        static MiscTab instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
private:
    MiscTab() = default;
    
    enum class SubTab { Movement, Logs, Skinchanger, InventoryUI, Lua };
    SubTab m_currentSubTab = SubTab::Movement;
    
    // References to feature configs
    features::misc::MovementExConfig& m_movementConfig = features::misc::MovementEx::Instance().GetConfig();
    features::misc::LogsConfig& m_logsConfig = features::misc::Logs::Instance().GetConfig();
    features::misc::SkinchangerConfig& m_skinchangerConfig = features::misc::Skinchanger::Instance().GetConfig();
    features::misc::InventoryUIConfig& m_inventoryUIConfig = features::misc::InventoryUI::Instance().GetConfig();
    
    void RenderMovementTab();
    void RenderLogsTab();
    void RenderSkinchangerTab();
    void RenderInventoryUITab();
    void RenderLuaTab();
    
    // Movement helpers
    void RenderAutoPeekSettings();
    void RenderThirdpersonSettings();
    void RenderFOVOverrideSettings();
    void RenderViewmodelChangerSettings();
    void RenderAspectRatioSettings();
    void RenderRecoilCrosshairSettings();
    void RenderPenetrationCrosshairSettings();
    
    // Logs helpers
    void RenderLogsSettings();
    
    // Skinchanger helpers
    void RenderSkinchangerSettings();
    void RenderWeaponSkinsList();
    void RenderKnifeGloveSettings();
    
    // Inventory UI helpers
    void RenderCaseOpeningSettings();
    void RenderStickerToolSettings();
    void RenderInspectPanelSettings();
    void RenderPatternSeedBrowserSettings();
    
    // Theme access
    gui::ViceTheme::Colors& m_colors = gui::ViceTheme::Instance().GetColors();
    gui::ViceWidgets& m_widgets = gui::ViceWidgets::Instance();
};

} // namespace gui