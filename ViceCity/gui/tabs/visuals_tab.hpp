#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include "../../features/visuals/esp.hpp"
#include "../../features/visuals/esp.hpp"
#include "../../features/visuals/esp.hpp"
#include "../../features/visuals/esp.hpp"
#include "../../features/visuals/esp.hpp"
#include "../../core/config/config_manager.hpp"
#include "../vice_widgets.hpp"
#include "../vice_theme.hpp"

namespace gui {

class VisualsTab {
public:
    static VisualsTab& Instance() {
        static VisualsTab instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
private:
    VisualsTab() = default;
    
    enum class SubTab { ESP, Chams, World, Radar, Effects };
    SubTab m_currentSubTab = SubTab::ESP;
    
    // References to feature configs
    features::visuals::ESPConfig& m_espConfig = features::visuals::ESP::Instance().GetConfig();
    features::visuals::ChamsConfig& m_chamsConfig = features::visuals::Chams::Instance().GetConfig();
    features::visuals::WorldConfig& m_worldConfig = features::visuals::World::Instance().GetConfig();
    features::visuals::RadarConfig& m_radarConfig = features::visuals::Radar::Instance().GetConfig();
    features::visuals::EffectsConfig& m_effectsConfig = features::visuals::Effects::Instance().GetConfig();
    
    void RenderESPTab();
    void RenderChamsTab();
    void RenderWorldTab();
    void RenderRadarTab();
    void RenderEffectsTab();
    
    // ESP helpers
    void RenderESPBoxSettings();
    void RenderESPSkeletonSettings();
    void RenderESPHealthSettings();
    void RenderESPArmorSettings();
    void RenderESPAmmoSettings();
    void RenderESPNameSettings();
    void RenderESPWeaponSettings();
    void RenderESPFlagsSettings();
    void RenderESPDormantSettings();
    void RenderESPSnaplinesSettings();
    void RenderESPOffscreenArrowsSettings();
    
    // Chams helpers
    void RenderChamsPlayerSettings();
    void RenderChamsArmsSettings();
    void RenderChamsWeaponsSettings();
    void RenderChamsAttachmentsSettings();
    
    // World helpers
    void RenderNightmodeSettings();
    void RenderPropTransparencySettings();
    void RenderGrenadePreviewSettings();
    void RenderHitmarkerSettings();
    void RenderDamageIndicatorSettings();
    void RenderSpreadCircleSettings();
    
    // Radar helpers
    void RenderRadarSettings();
    
    // Effects helpers
    void RenderBulletTracersSettings();
    void RenderImpactBeamsSettings();
    void RenderBulletBeamsSettings();
    void RenderHitNumbersSettings();
    
    // Theme access
    gui::ViceTheme::Colors& m_colors = gui::ViceTheme::Instance().GetColors();
    gui::ViceWidgets& m_widgets = gui::ViceWidgets::Instance();
};

} // namespace gui