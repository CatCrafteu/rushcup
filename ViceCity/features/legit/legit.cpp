#include "core/pch.hpp"
#include "features/legit/triggerbot.hpp"
#include "features/legit/legit_aa.hpp"
#include "features/legit/movement.hpp"
#include "core/sdk/interfaces.hpp"
#include "core/config/config_manager.hpp"
#include "core/logger/logger.hpp"

namespace features::legit {

// ========== Triggerbot ==========

void Triggerbot::Initialize() {
    LoadConfig();
    LOG_INFO(Legit, "Triggerbot initialized");
}

void Triggerbot::Shutdown() {
    SaveConfig();
    LOG_INFO(Legit, "Triggerbot shutdown");
}

void Triggerbot::OnCreateMove(sdk::CUserCmd* cmd) {
    if (!m_config.enabled) return;
    if (!CheckKeybind()) return;
    if (!CheckConditions(cmd)) return;
    
    auto local = GetLocalPlayer();
    if (!local) return;
    
    auto weapon = GetActiveWeapon();
    if (!weapon) return;
    
    // Trace from eye position
    sdk::Vector3D eyePos;
    sdk::Vector3D forward = sdk::AngleVectors(cmd->view_angles);
    sdk::Vector3D end = eyePos + forward * 8192.0f;
    
    sdk::Ray_t ray;
    ray.Init(eyePos, end);
    
    sdk::Trace_t trace;
    sdk::TraceFilter filter;
    filter.skip = local;
    
    sdk::g_pEngineTrace->TraceRay(ray, 0x46004003, &filter, &trace);
    
    if (!trace.hit || !trace.entity) return;
    
    auto targetEntity = static_cast<sdk::CBaseEntity*>(trace.entity);
    if (!targetEntity) return;
    
    int hitgroup = trace.hitgroup;
    if (!CheckHitgroup(hitgroup)) return;
    
    if (m_config.checkVisible && trace.fraction < 1.0f) return;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastShotTime).count();
    
    if (elapsed < m_triggerDelay) return;
    
    if (m_burstShotsFired < m_config.burstShots) {
        cmd->buttons |= 1;
        m_burstShotsFired++;
        m_lastShotTime = now;
        
        if (m_burstShotsFired >= m_config.burstShots) {
            m_burstDelayTimer = m_config.burstDelay;
        }
    } else if (m_burstDelayTimer > 0) {
        m_burstDelayTimer--;
        if (m_burstDelayTimer == 0) {
            m_burstShotsFired = 0;
            m_triggerDelay = m_config.delayMin + (rand() % (m_config.delayMax - m_config.delayMin + 1));
        }
    }
}

bool Triggerbot::CheckKeybind() {
    if (m_config.keybind.empty()) return true;
    
    int key = 0;
    bool pressed = (GetAsyncKeyState(key) & 0x8000) != 0;
    
    using KeybindMode = TriggerbotConfig::KeybindMode;
    switch (m_config.keybindMode) {
        case KeybindMode::Toggle:
            if (pressed && !m_wasTriggered) {
                m_wasTriggered = true;
                m_config.enabled = !m_config.enabled;
            } else if (!pressed) {
                m_wasTriggered = false;
            }
            break;
        case KeybindMode::Hold:
            return pressed;
        case KeybindMode::DoubleTap:
            return pressed;
    }
    
    return m_config.enabled;
}

bool Triggerbot::CheckConditions(sdk::CUserCmd* cmd) {
    return true;
}

bool Triggerbot::CheckHitgroup(int hitgroup) {
    for (int hg : m_config.hitgroups) {
        if (hg == hitgroup) return true;
    }
    return false;
}

int Triggerbot::GetBestHitgroup(sdk::CBaseEntity* target) {
    return 1;
}

bool Triggerbot::IsVisible(sdk::CBaseEntity* target, int hitbox) {
    return true;
}

void Triggerbot::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("aimbot_legit.triggerbot", key, def);
        if (opt) value = *opt;
    };
    
    load("enabled", m_config.enabled, false);
    load("keybind", m_config.keybind, std::string(""));
    load("delay_min", m_config.delayMin, 0);
    load("delay_max", m_config.delayMax, 50);
    load("burst_shots", m_config.burstShots, 1);
    load("burst_delay", m_config.burstDelay, 0);
}

void Triggerbot::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("aimbot_legit.triggerbot", key, value);
    };
    
    save("enabled", m_config.enabled);
    save("keybind", m_config.keybind);
    save("delay_min", m_config.delayMin);
    save("delay_max", m_config.delayMax);
    save("burst_shots", m_config.burstShots);
    save("burst_delay", m_config.burstDelay);
}

sdk::CBaseEntity* Triggerbot::GetLocalPlayer() const {
    int index = sdk::g_pEngine->GetLocalPlayer();
    if (index <= 0) return nullptr;
    return static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(index));
}

sdk::CBaseWeapon* Triggerbot::GetActiveWeapon() const {
    return nullptr;
}

// ========== Backtrack ==========

void Backtrack::Initialize() {
    LoadConfig();
    LOG_INFO(Legit, "Backtrack initialized");
}

void Backtrack::Shutdown() {
    SaveConfig();
    LOG_INFO(Legit, "Backtrack shutdown");
}

void Backtrack::OnCreateMove(sdk::CUserCmd* cmd) {
    if (!m_config.enabled) return;
}

void Backtrack::OnFrameStageNotify(int stage) {
    if (stage != 0) return;
    UpdateRecords(stage);
}

std::optional<Backtrack::BacktrackRecord> Backtrack::GetBestRecord(sdk::CBaseEntity* entity) {
    int index = 0;
    if (index < 0 || index >= 64) return std::nullopt;
    
    auto& records = m_records[index];
    if (records.empty()) return std::nullopt;
    
    for (auto it = records.rbegin(); it != records.rend(); ++it) {
        if (IsRecordValid(*it, entity)) {
            return *it;
        }
    }
    
    return std::nullopt;
}

std::vector<Backtrack::BacktrackRecord> Backtrack::GetRecords(sdk::CBaseEntity* entity) {
    int index = 0;
    if (index < 0 || index >= 64) return {};
    
    std::vector<BacktrackRecord> result;
    for (auto& r : m_records[index]) {
        if (r.valid) result.push_back(r);
    }
    return result;
}

void Backtrack::ApplyBacktrack(sdk::CUserCmd* cmd, sdk::CBaseEntity* target, const BacktrackRecord& record) {
}

void Backtrack::RenderBacktrack() {
    if (!m_config.visualize) return;
}

void Backtrack::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("aimbot_legit.backtrack", key, def);
        if (opt) value = *opt;
    };
    
    load("enabled", m_config.enabled, false);
    load("time_limit", m_config.timeLimit, 200);
    load("visualize", m_config.visualize, false);
}

void Backtrack::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("aimbot_legit.backtrack", key, value);
    };
    
    save("enabled", m_config.enabled);
    save("time_limit", m_config.timeLimit);
    save("visualize", m_config.visualize);
}

void Backtrack::UpdateRecords(int stage) {
    int maxClients = sdk::g_pEngine->GetMaxClients();
    
    for (int i = 1; i <= maxClients; ++i) {
        if (i == sdk::g_pEngine->GetLocalPlayer()) continue;
        
        auto entity = static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(i));
        if (!entity) {
            m_records[i].clear();
            continue;
        }
        
        BacktrackRecord record;
        record.simulationTime = 0.0f;
        record.tickCount = sdk::g_pGlobalVars->tick_count;
        record.valid = true;
        
        m_records[i].push_front(record);
    }
    
    PurgeOldRecords();
}

void Backtrack::PurgeOldRecords() {
    float maxTime = m_config.timeLimit / 1000.0f;
    int currentTick = sdk::g_pGlobalVars->tick_count;
    
    for (int i = 0; i < 64; ++i) {
        auto& records = m_records[i];
        while (!records.empty()) {
            auto& front = records.front();
            float age = (currentTick - front.tickCount) * sdk::g_pGlobalVars->interval_per_tick;
            if (age > maxTime || records.size() > MAX_RECORDS) {
                records.pop_front();
            } else break;
        }
    }
}

bool Backtrack::IsRecordValid(const BacktrackRecord& record, sdk::CBaseEntity* entity) {
    return record.valid && record.simulationTime > 0.0f;
}

float Backtrack::GetLerpTime() {
    return 0.0f;
}

// ========== LegitAA ==========

void LegitAA::Initialize() {
    LoadConfig();
    LOG_INFO(Legit, "LegitAA initialized");
}

void LegitAA::Shutdown() {
    SaveConfig();
    LOG_INFO(Legit, "LegitAA shutdown");
}

void LegitAA::OnCreateMove(sdk::CUserCmd* cmd) {
    if (!m_config.enabled) return;
    if (!CheckKeybind()) return;
    
    using Pitch = LegitAAConfig::Pitch;
    using Yaw = LegitAAConfig::Yaw;
    using LBYMode = LegitAAConfig::LBYMode;
    
    if (m_config.pitch != Pitch::Off) {
    }
    
    if (m_config.yaw != Yaw::Off) {
        if (m_config.yaw == Yaw::Freestanding) {
            DoFreestanding(cmd);
        } else if (m_config.yaw == Yaw::Jitter) {
            DoJitterYaw(cmd);
        } else if (m_config.yaw == Yaw::Static) {
            DoStaticYaw(cmd);
        }
    }
    
    if (m_config.lbyMode != LBYMode::Off) {
        DoLBY(cmd);
    }
}

bool LegitAA::CheckKeybind() {
    if (m_config.keybind.empty()) return true;
    
    int key = 0;
    bool pressed = (GetAsyncKeyState(key) & 0x8000) != 0;
    
    using KeybindMode = LegitAAConfig::KeybindMode;
    switch (m_config.keybindMode) {
        case KeybindMode::Toggle:
            if (pressed && !m_lastKeyState) {
                m_lastKeyState = true;
                m_config.enabled = !m_config.enabled;
            } else if (!pressed) {
                m_lastKeyState = false;
            }
            break;
        case KeybindMode::Hold:
            return pressed;
    }
    
    return m_config.enabled;
}

void LegitAA::DoFreestanding(sdk::CUserCmd* cmd) {
}

void LegitAA::DoEdgeYaw(sdk::CUserCmd* cmd) {
}

void LegitAA::DoStaticYaw(sdk::CUserCmd* cmd) {
    cmd->view_angles.yaw = m_config.yawStatic;
}

void LegitAA::DoJitterYaw(sdk::CUserCmd* cmd) {
    static bool jitter = false;
    jitter = !jitter;
    cmd->view_angles.yaw += jitter ? m_config.yawJitterRange : -m_config.yawJitterRange;
}

void LegitAA::DoLBY(sdk::CUserCmd* cmd) {
}

void LegitAA::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("aimbot_legit.legit_aa", key, def);
        if (opt) value = *opt;
    };
    
    load("enabled", m_config.enabled, false);
}

void LegitAA::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("aimbot_legit.legit_aa", key, value);
    };
    
    save("enabled", m_config.enabled);
}

bool LegitAA::IsKeyPressed(const std::string& keybind) {
    if (keybind.empty()) return false;
    int key = 0;
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

// ========== Movement ==========

void Movement::Initialize() {
    LoadConfig();
    LOG_INFO(Legit, "Movement initialized");
}

void Movement::Shutdown() {
    SaveConfig();
    LOG_INFO(Legit, "Movement shutdown");
}

void Movement::OnCreateMove(sdk::CUserCmd* cmd) {
    if (m_config.bhopEnabled) DoBhop(cmd);
    if (m_config.autoStrafeEnabled) DoAutoStrafe(cmd);
    if (m_config.edgeJumpEnabled) DoEdgeJump(cmd);
    if (m_config.fastDuckEnabled) DoFastDuck(cmd);
    if (m_config.slowWalkEnabled) DoSlowWalk(cmd);
    if (m_config.autoPeekEnabled) DoAutoPeek(cmd);
}

void Movement::DoBhop(sdk::CUserCmd* cmd) {
    if (!IsOnGround()) {
        cmd->buttons &= ~4;
    }
}

void Movement::DoAutoStrafe(sdk::CUserCmd* cmd) {
    if (!IsOnGround()) {
        if (cmd->mouse_dx > 0) cmd->side_move = 450.0f;
        else if (cmd->mouse_dx < 0) cmd->side_move = -450.0f;
    }
}

void Movement::DoEdgeJump(sdk::CUserCmd* cmd) {
    if (!IsKeyPressed(m_config.edgeJumpKeybind)) return;
    
    if (IsOnGround()) {
        cmd->buttons |= 4;
    }
}

void Movement::DoFastDuck(sdk::CUserCmd* cmd) {
}

void Movement::DoSlowWalk(sdk::CUserCmd* cmd) {
    if (!IsKeyPressed(m_config.slowWalkKeybind)) return;
    
    float maxSpeed = GetMaxSpeed() * (m_config.slowWalkSpeed / 100.0f);
    sdk::Vector3D move = {cmd->forward_move, cmd->side_move, 0};
    float speed = move.Length2D();
    
    if (speed > maxSpeed) {
        float ratio = maxSpeed / speed;
        cmd->forward_move *= ratio;
        cmd->side_move *= ratio;
    }
}

void Movement::DoAutoPeek(sdk::CUserCmd* cmd) {
    if (!IsKeyPressed(m_config.autoPeekKeybind)) {
        if (m_wasAutoPeeking) {
            m_wasAutoPeeking = false;
        }
        return;
    }
    
    if (!m_wasAutoPeeking) {
        auto local = GetLocalPlayer();
        if (local) m_autoPeekStartPos = {};
        m_wasAutoPeeking = true;
    }
}

bool Movement::IsOnGround() {
    return true;
}

float Movement::GetMaxSpeed() {
    return 250.0f;
}

bool Movement::IsKeyPressed(const std::string& keybind) {
    if (keybind.empty()) return false;
    int key = 0;
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

void Movement::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("aimbot_legit.movement", key, def);
        if (opt) value = *opt;
    };
    
    load("bhop_enabled", m_config.bhopEnabled, false);
}

void Movement::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("aimbot_legit.movement", key, value);
    };
    
    save("bhop_enabled", m_config.bhopEnabled);
}

sdk::CBaseEntity* Movement::GetLocalPlayer() const {
    int index = sdk::g_pEngine->GetLocalPlayer();
    if (index <= 0) return nullptr;
    return static_cast<sdk::CBaseEntity*>(sdk::g_pEntityList->GetClientEntity(index));
}

} // namespace features::legit