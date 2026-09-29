#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include "../../features/rage/rage_aimbot.hpp"
#include "../../features/rage/exploitation.hpp"
#include "../../features/rage/anti_aim.hpp"
#include "../../features/rage/resolver.hpp"
#include "../../features/rage/autowall.hpp"
#include "../../core/config/config_manager.hpp"
#include "../vice_widgets.hpp"
#include "../vice_theme.hpp"

namespace gui {

class RageBotTab {
public:
    static RageBotTab& Instance() {
        static RageBotTab instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
private:
    RageBotTab() = default;
    
    enum class SubTab { Aim, Exploits, AntiAim, Resolver, Logs };
    SubTab m_currentSubTab = SubTab::Aim;
    
    // References to feature configs
    features::rage::RageAimbotConfig& m_aimConfig = features::rage::RageAimbot::Instance().GetConfig();
    features::rage::ExploitationConfig& m_exploitConfig = features::rage::Exploitation::Instance().GetConfig();
    features::rage::AntiAimConfig& m_aaConfig = features::rage::AntiAim::Instance().GetConfig();
    features::rage::ResolverConfig& m_resolverConfig = features::rage::Resolver::Instance().GetConfig();
    
    void RenderAimTab();
    void RenderExploitsTab();
    void RenderAntiAimTab();
    void RenderResolverTab();
    void RenderLogsTab();
    
    // Helper functions
    void RenderHitboxPriority();
    void RenderMultipointSettings();
    void RenderHitchanceSettings();
    void RenderMinimumDamage();
    void RenderAutowallSettings();
    void RenderPreferBodyAim();
    void RenderDoubleTap();
    void RenderHideShots();
    void RenderFakeLag();
    void RenderQuickStop();
    void RenderQuickPeek();
    void RenderPitchSettings();
    void RenderYawSettings();
    void RenderDesyncSettings();
    void RenderLBYBreaker();
    void RenderFreestanding();
    void RenderManualAA();
    void RenderResolverSettings();
    void RenderAnimFixSettings();
    void RenderLogSettings();
    
    // Theme access
    gui::ViceTheme::Colors& m_colors = gui::ViceTheme::Instance().GetColors();
    gui::ViceWidgets& m_widgets = gui::ViceWidgets::Instance();
};

} // namespace gui