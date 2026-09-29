#include "core/pch.hpp"
#include "features/rage/anti_aim.hpp"
#include "core/sdk/interfaces.hpp"
#include "core/config/config_manager.hpp"
#include "core/logger/logger.hpp"

namespace features::rage {

void AntiAim::Initialize() {
    LoadConfig();
    LOG_INFO(Rage, "AntiAim initialized");
}

void AntiAim::Shutdown() {
    SaveConfig();
    LOG_INFO(Rage, "AntiAim shutdown");
}

void AntiAim::OnCreateMove(sdk::CUserCmd* cmd) {
    if (!m_config.enabled) return;
    
    auto local = GetLocalPlayer();
    if (!local) return;
    
    // Apply pitch
    switch (m_config.pitch) {
        case AntiAimConfig::Pitch::Down:
            cmd->view_angles.pitch = 89.0f;
            break;
        case AntiAimConfig::Pitch::Up:
            cmd->view_angles.pitch = -89.0f;
            break;
        case AntiAimConfig::Pitch::Zero:
            cmd->view_angles.pitch = 0.0f;
            break;
        case AntiAimConfig::Pitch::Jitter:
            cmd->view_angles.pitch = (rand() % 2 == 0) ? 89.0f : -89.0f;
            break;
        case AntiAimConfig::Pitch::Custom:
            cmd->view_angles.pitch = m_config.pitchCustom;
            break;
    }
    
    // Apply yaw base
    float baseYaw = 0.0f;
    switch (m_config.yawBase) {
        case AntiAimConfig::YawBase::LocalView:
            baseYaw = cmd->view_angles.yaw;
            break;
        case AntiAimConfig::YawBase::AtTargets:
            baseYaw = GetAtTargetsYaw();
            break;
        case AntiAimConfig::YawBase::Freestanding:
            baseYaw = GetFreestandingYaw();
            break;
    }
    
    // Apply yaw
    switch (m_config.yaw) {
        case AntiAimConfig::Yaw::Backward:
            cmd->view_angles.yaw = baseYaw + 180.0f;
            break;
        case AntiAimConfig::Yaw::Jitter:
            cmd->view_angles.yaw = baseYaw + 180.0f + (rand() % 2 == 0 ? m_config.yawJitterRange : -m_config.yawJitterRange);
            break;
        case AntiAimConfig::Yaw::Spin:
            cmd->view_angles.yaw = baseYaw + 180.0f + fmodf(sdk::g_pGlobalVars->current_time * m_config.yawSpinSpeed * 360.0f, 360.0f);
            break;
        case AntiAimConfig::Yaw::Static:
            cmd->view_angles.yaw = baseYaw + m_config.yawStatic;
            break;
        case AntiAimConfig::Yaw::Custom:
            // Custom yaw logic
            break;
    }
    
    // Apply desync
    ApplyDesync(cmd);
    
    // Apply LBY breaker
    if (m_config.lbyBreaker) {
        ApplyLBYBreaker(cmd);
    }
    
    // Apply freestanding
    if (m_config.freestanding) {
        ApplyFreestanding(cmd);
    }
    
    // Apply manual AA
    ApplyManualAA(cmd);
    
    cmd->view_angles.Normalize();
}

void AntiAim::ApplyDesync(sdk::CUserCmd* cmd) {
    if (m_config.desync == AntiAimConfig::Desync::Off) return;
    
    static bool desyncSide = false;
    
    switch (m_config.desync) {
        case AntiAimConfig::Desync::Static:
            // Static desync
            break;
        case AntiAimConfig::Desync::Jitter:
            desyncSide = !desyncSide;
            break;
        case AntiAimConfig::Desync::AvoidOverlap:
            // Avoid overlap with LBY
            break;
    }
    
    // Apply desync by modifying command or using exploit
}

void AntiAim::ApplyLBYBreaker(sdk::CUserCmd* cmd) {
    // LBY breaker logic
}

void AntiAim::ApplyFreestanding(sdk::CUserCmd* cmd) {
    // Freestanding logic
}

void AntiAim::ApplyManualAA(sdk::CUserCmd* cmd) {
    // Manual anti-aim
    bool left = false, right = false, back = false; // Check keybinds
    
    if (left) {
        cmd->view_angles.yaw += 90.0f;
    } else if (right) {
        cmd->view_angles.yaw -= 90.0f;
    } else if (back) {
        cmd->view_angles.yaw += 180.0f;
    }
}

float AntiAim::GetFreestandingYaw() {
    // Calculate freestanding yaw by tracing left/right
    return 0.0f;
}

float AntiAim::GetAtTargetsYaw() {
    // Calculate yaw to nearest target
    auto local = GetLocalPlayer();
    if (!local) return 0.0f;
    // Would find nearest enemy and calculate angle
    return 0.0f;
}

void AntiAim::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("anti_aim", key, def);
        if (opt) value = *opt;
    };
    
    load("enabled", m_config.enabled, true);
    load("pitch", m_config.pitch, AntiAimConfig::Pitch::Down);
    load("yaw_base", m_config.yawBase, AntiAimConfig::YawBase::AtTargets);
    load("yaw", m_config.yaw, AntiAimConfig::Yaw::Backward);
    load("desync", m_config.desync, AntiAimConfig::Desync::Jitter);
    load("lby_breaker", m_config.lbyBreaker, true);
    load("freestanding", m_config.freestanding, true);
}

void AntiAim::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("anti_aim", key, value);
    };
    
    save("enabled", m_config.enabled);
    save("pitch", m_config.pitch);
    save("yaw_base", m_config.yawBase);
    save("yaw", m_config.yaw);
    save("desync", m_config.desync);
    save("lby_breaker", m_config.lbyBreaker);
    save("freestanding", m_config.freestanding);
}

sdk::CBaseEntity* AntiAim::GetLocalPlayer() const {
    int index = sdk::g_pEngine->GetLocalPlayer();
    if (index <= 0) return nullptr;
    return static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(index));
}

} // namespace features::rage