#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include "../../features/legit/triggerbot.hpp"
#include "../../features/legit/triggerbot.hpp"
#include "../../features/legit/legit_aa.hpp"
#include "../../features/legit/movement.hpp"
#include "../../core/config/config_manager.hpp"
#include "../vice_widgets.hpp"
#include "../vice_theme.hpp"

namespace gui {

class LegitBotTab {
public:
    static LegitBotTab& Instance() {
        static LegitBotTab instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
private:
    LegitBotTab() = default;
    
    enum class SubTab { Triggerbot, Backtrack, LegitAA, Movement };
    SubTab m_currentSubTab = SubTab::Triggerbot;
    
    // References to feature configs
    features::legit::TriggerbotConfig& m_triggerConfig = features::legit::Triggerbot::Instance().GetConfig();
    features::legit::BacktrackConfig& m_backtrackConfig = features::legit::Backtrack::Instance().GetConfig();
    features::legit::LegitAAConfig& m_legitAAConfig = features::legit::LegitAA::Instance().GetConfig();
    features::legit::MovementConfig& m_movementConfig = features::legit::Movement::Instance().GetConfig();
    
    void RenderTriggerbotTab();
    void RenderBacktrackTab();
    void RenderLegitAATab();
    void RenderMovementTab();
    
    // Helper functions
    void RenderTriggerbotSettings();
    void RenderTriggerbotHitgroups();
    void RenderTriggerbotChecks();
    void RenderBacktrackSettings();
    void RenderBacktrackVisualization();
    void RenderLegitAAPitch();
    void RenderLegitAAYaw();
    void RenderLegitAALBY();
    void RenderBhopSettings();
    void RenderAutoStrafeSettings();
    void RenderEdgeJumpSettings();
    void RenderFastDuckSettings();
    void RenderSlowWalkSettings();
    void RenderAutoPeekSettings();
    
    // Theme access
    gui::ViceTheme::Colors& m_colors = gui::ViceTheme::Instance().GetColors();
    gui::ViceWidgets& m_widgets = gui::ViceWidgets::Instance();
};

} // namespace gui