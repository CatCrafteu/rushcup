#include "core/pch.hpp"
#include "resolver/resolver.hpp"
#include "resolver/animfix.hpp"
#include "resolver/lagcomp.hpp"
#include "core/sdk/interfaces.hpp"
#include "core/config/config_manager.hpp"
#include "core/logger/logger.hpp"

namespace resolver {

// ========== Resolver ==========

void Resolver::Initialize() {
    LoadConfig();
    LOG_INFO(Resolver, "Resolver initialized");
}

void Resolver::Shutdown() {
    SaveConfig();
    LOG_INFO(Resolver, "Resolver shutdown");
}

void Resolver::OnCreateMove(sdk::CUserCmd* cmd) {
    if (!m_config.enabled) return;
}

void Resolver::OnFrameStageNotify(int stage) {
    if (stage != 0) return;
    UpdateRecords(stage);
}

std::optional<Resolver::PlayerRecord> Resolver::GetRecord(int index) {
    if (index < 0 || index >= 64) return std::nullopt;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& record = m_records[index];
    if (!record.valid) return std::nullopt;
    return record;
}

void Resolver::ResolvePlayer(sdk::CBaseEntity* entity, PlayerRecord& record) {
    if (!m_config.enabled) return;
    
    switch (m_config.mode) {
        case ResolverConfig::Mode::Brute:
            BruteForceResolve(record);
            break;
        case ResolverConfig::Mode::History:
            HistoryResolve(record);
            break;
        case ResolverConfig::Mode::ML:
            MLResolve(record);
            break;
    }
    
    if (m_config.animFix.enabled) {
        LBYResolve(record);
    }
    
    FreestandingResolve(record);
}

float Resolver::GetResolvedYaw(int index) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index < 0 || index >= 64) return 0.0f;
    return m_records[index].resolvedYaw;
}

float Resolver::GetResolvedPitch(int index) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index < 0 || index >= 64) return 0.0f;
    return m_records[index].resolvedPitch;
}

void Resolver::BruteForceResolve(PlayerRecord& record) {
    static int bruteStage = 0;
    record.resolvedYaw = BRUTE_STAGES[bruteStage % NUM_BRUTE_STAGES];
    record.resolveMode = PlayerRecord::ResolveMode::Brute;
}

void Resolver::HistoryResolve(PlayerRecord& record) {
    if (record.yawHistory.empty()) {
        record.resolvedYaw = record.eyeAngles.yaw;
        return;
    }
    
    float sum = 0.0f;
    int count = 0;
    for (auto it = record.yawHistory.rbegin(); it != record.yawHistory.rend() && count < 10; ++it, ++count) {
        sum += *it;
    }
    
    if (count > 0) {
        record.resolvedYaw = sum / count;
    } else {
        record.resolvedYaw = record.eyeAngles.yaw;
    }
    
    record.resolveMode = PlayerRecord::ResolveMode::History;
}

void Resolver::MLResolve(PlayerRecord& record) {
    record.resolvedYaw = record.eyeAngles.yaw;
    record.resolveMode = PlayerRecord::ResolveMode::ML;
}

void Resolver::LBYResolve(PlayerRecord& record) {
    float lbyDelta = CalculateLBYDelta(record);
    record.lbyDelta = lbyDelta;
    
    if (std::abs(lbyDelta) > 35.0f) {
        record.resolvedYaw = record.lowerBodyYawTarget;
        record.resolveMode = PlayerRecord::ResolveMode::LBY;
    }
}

void Resolver::FreestandingResolve(PlayerRecord& record) {
    record.resolvedYaw = GetAtTargetsYaw(nullptr);
    record.resolveMode = PlayerRecord::ResolveMode::Freestanding;
}

void Resolver::LogMiss(int index, float resolvedYaw, float actualYaw) {
    if (!m_config.logMisses) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index >= 0 && index < 64) {
        m_records[index].shotsMissed++;
        LOG_DEBUG(Resolver, "Miss logged for player {}: resolved={:.1f}, actual={:.1f}", index, resolvedYaw, actualYaw);
    }
}

void Resolver::LogHit(int index) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index >= 0 && index < 64) {
        m_records[index].shotsHit++;
    }
}

bool Resolver::IsResolving(int index) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index < 0 || index >= 64) return false;
    return m_records[index].valid && m_records[index].resolveMode != PlayerRecord::ResolveMode::None;
}

void Resolver::ResetPlayer(int index) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index >= 0 && index < 64) {
        m_records[index] = PlayerRecord{};
    }
}

void Resolver::ResetAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& record : m_records) {
        record = PlayerRecord{};
    }
}

void Resolver::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("resolver", key, def);
        if (opt) value = *opt;
    };
    
    load("enabled", m_config.enabled, true);
    load("log_misses", m_config.logMisses, true);
    load("update_rate", m_config.updateRate, 1);
    load("max_history", m_config.maxHistory, 128);
}

void Resolver::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("resolver", key, value);
    };
    
    save("enabled", m_config.enabled);
    save("log_misses", m_config.logMisses);
    save("update_rate", m_config.updateRate);
    save("max_history", m_config.maxHistory);
}

void Resolver::UpdateRecords(int stage) {
    int maxClients = sdk::interfaces::g_pEngine->GetMaxClients();
    
    for (int i = 1; i <= maxClients; ++i) {
        if (i == sdk::interfaces::g_pEngine->GetLocalPlayer()) continue;
        
        auto entity = static_cast<sdk::CBaseEntity*>(sdk::interfaces::g_pEntityList->GetClientEntity(i));
        if (!entity) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_records[i] = PlayerRecord{};
            continue;
        }
        
        PlayerRecord& record = m_records[i];
        record.entity = entity;
        
        StoreLayerData(entity, record);
        
        record.eyeAngles = {};
        record.absAngles = {};
        record.origin = {};
        record.velocity = {};
        record.simulationTime = 0.0f;
        record.lowerBodyYawTarget = 0.0f;
        record.flags = 0;
        record.tickCount = sdk::interfaces::g_pGlobalVars->tick_count;
        record.valid = true;
        record.dormant = false;
        
        UpdateYawHistory(record);
        UpdateLBYHistory(record);
    }
}

void Resolver::StoreLayerData(sdk::CBaseEntity* entity, PlayerRecord& record) {
    for (int i = 0; i < 13; ++i) {
        record.layers[i].sequence = 0;
        record.layers[i].cycle = 0.0f;
        record.layers[i].weight = 0.0f;
        record.layers[i].playbackRate = 0.0f;
        record.layers[i].order = 0;
    }
}

void Resolver::StorePoseParams(sdk::CBaseEntity* entity, PlayerRecord& record) {
}

float Resolver::CalculateLBYDelta(const PlayerRecord& record) {
    float delta = record.eyeAngles.yaw - record.lowerBodyYawTarget;
    while (delta > 180.0f) delta -= 360.0f;
    while (delta < -180.0f) delta += 360.0f;
    return delta;
}

bool Resolver::IsEntityMoving(const PlayerRecord& record) {
    return record.velocity.Length2D() > 0.1f;
}

float Resolver::GetFreestandingYaw(sdk::CBaseEntity* entity, const PlayerRecord& record) {
    return record.eyeAngles.yaw;
}

float Resolver::GetAtTargetsYaw(sdk::CBaseEntity* entity) {
    return 0.0f;
}

void Resolver::UpdateYawHistory(PlayerRecord& record) {
    record.yawHistory.push_back(record.eyeAngles.yaw);
    if (record.yawHistory.size() > 64) {
        record.yawHistory.pop_front();
    }
}

void Resolver::UpdateLBYHistory(PlayerRecord& record) {
    record.lbyHistory.push_back(record.lowerBodyYawTarget);
    if (record.lbyHistory.size() > 64) {
        record.lbyHistory.pop_front();
    }
}

int Resolver::GetMissedShots(int index) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index < 0 || index >= 64) return 0;
    return m_records[index].shotsMissed;
}

// ========== AnimFix ==========

void AnimFix::Initialize() {
    LoadConfig();
    LOG_INFO(Resolver, "AnimFix initialized");
}

void AnimFix::Shutdown() {
    SaveConfig();
    LOG_INFO(Resolver, "AnimFix shutdown");
}

void AnimFix::OnFrameStageNotify(int stage) {
    if (stage != 0) return;
    
    int maxClients = sdk::interfaces::g_pEngine->GetMaxClients();
    
    for (int i = 1; i <= maxClients; ++i) {
        if (i == sdk::interfaces::g_pEngine->GetLocalPlayer()) continue;
        
        auto entity = static_cast<sdk::CBaseEntity*>(sdk::interfaces::g_pEntityList->GetClientEntity(i));
        if (!entity) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_animData[i] = PlayerAnimData{};
            continue;
        }
        
        StoreAnimData(entity, m_animData[i]);
        
        if (m_config.enabled) {
            ApplyAnimFix(entity, m_animData[i]);
        }
    }
}

std::optional<AnimFix::PlayerAnimData> AnimFix::GetAnimData(int index) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index < 0 || index >= 64) return std::nullopt;
    if (!m_animData[index].valid) return std::nullopt;
    return m_animData[index];
}

void AnimFix::FixAnimation(sdk::CBaseEntity* entity) {
}

void AnimFix::PredictAnimation(sdk::CBaseEntity* entity, PlayerAnimData& data) {
    if (!m_config.prediction) return;
}

void AnimFix::UpdateLayers(sdk::CBaseEntity* entity, PlayerAnimData& data) {
    if (!m_config.layers) return;
}

void AnimFix::UpdatePoseParams(sdk::CBaseEntity* entity, PlayerAnimData& data) {
}

void AnimFix::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("resolver.animfix", key, def);
        if (opt) value = *opt;
    };
    
    load("enabled", m_config.enabled, true);
    load("layers", m_config.layers, true);
    load("prediction", m_config.prediction, true);
}

void AnimFix::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("resolver.animfix", key, value);
    };
    
    save("enabled", m_config.enabled);
    save("layers", m_config.layers);
    save("prediction", m_config.prediction);
}

void AnimFix::StoreAnimData(sdk::CBaseEntity* entity, PlayerAnimData& data) {
    for (int i = 0; i < 13; ++i) {
        data.serverLayers[i] = AnimationLayer{};
        data.clientLayers[i] = AnimationLayer{};
    }
    
    for (int i = 0; i < 24; ++i) {
        data.poseParams[i] = 0.0f;
    }
    
    data.eyeAngles = {};
    data.absAngles = {};
    data.origin = {};
    data.velocity = {};
    data.simulationTime = 0.0f;
    data.lowerBodyYawTarget = 0.0f;
    data.duckAmount = 0.0f;
    data.duckSpeed = 0.0f;
    data.flags = 0;
    data.valid = true;
    data.dormant = false;
    data.tickCount = sdk::interfaces::g_pGlobalVars->tick_count;
}

void AnimFix::ApplyAnimFix(sdk::CBaseEntity* entity, PlayerAnimData& data) {
    FixLayerWeights(entity, data);
    FixLayerCycles(entity, data);
    FixPoseParams(entity, data);
}

void AnimFix::FixLayerWeights(sdk::CBaseEntity* entity, PlayerAnimData& data) {
}

void AnimFix::FixLayerCycles(sdk::CBaseEntity* entity, PlayerAnimData& data) {
}

void AnimFix::FixPoseParams(sdk::CBaseEntity* entity, PlayerAnimData& data) {
}

bool AnimFix::IsLayerValid(const AnimationLayer& layer) {
    return layer.weight > 0.0f && layer.playbackRate > 0.0f;
}

// ========== LagComp ==========

void LagComp::Initialize() {
    LoadConfig();
    LOG_INFO(Resolver, "LagComp initialized");
}

void LagComp::Shutdown() {
    SaveConfig();
    LOG_INFO(Resolver, "LagComp shutdown");
}

void LagComp::OnCreateMove(sdk::CUserCmd* cmd) {
    if (!m_config.enabled) return;
}

void LagComp::OnFrameStageNotify(int stage) {
    if (stage != 0) return;
    UpdateRecords(stage);
}

std::optional<LagComp::LagRecord> LagComp::GetBestRecord(sdk::CBaseEntity* entity) {
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

std::vector<LagComp::LagRecord> LagComp::GetRecords(sdk::CBaseEntity* entity) {
    int index = 0;
    if (index < 0 || index >= 64) return {};
    
    std::vector<LagRecord> result;
    for (auto& r : m_records[index]) {
        if (r.valid) result.push_back(r);
    }
    return result;
}

void LagComp::ApplyLagCompensation(sdk::CBaseEntity* entity, const LagRecord& record) {
}

void LagComp::RestoreEntity(sdk::CBaseEntity* entity) {
}

void LagComp::RenderRecords() {
}

void LagComp::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto load = [&](const char* key, auto& value, const auto& def) {
        auto opt = config.Get("resolver.lagcomp", key, def);
        if (opt) value = *opt;
    };
    
    load("enabled", m_config.enabled, true);
    load("max_records", m_config.maxRecords, 64);
    load("max_time", m_config.maxTime, 0.2f);
}

void LagComp::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto save = [&](const char* key, const auto& value) {
        config.Set("resolver.lagcomp", key, value);
    };
    
    save("enabled", m_config.enabled);
    save("max_records", m_config.maxRecords);
    save("max_time", m_config.maxTime);
}

void LagComp::UpdateRecords(int stage) {
    int maxClients = sdk::interfaces::g_pEngine->GetMaxClients();
    
    for (int i = 1; i <= maxClients; ++i) {
        if (i == sdk::interfaces::g_pEngine->GetLocalPlayer()) continue;
        
        auto entity = static_cast<sdk::CBaseEntity*>(sdk::interfaces::g_pEntityList->GetClientEntity(i));
        if (!entity) {
            m_records[i].clear();
            continue;
        }
        
        LagRecord record;
        record.origin = {};
        record.angles = {};
        record.velocity = {};
        record.eyeAngles = {};
        record.simulationTime = 0.0f;
        record.flags = 0;
        record.tickCount = sdk::interfaces::g_pGlobalVars->tick_count;
        record.valid = true;
        record.shot = false;
        
        StoreBoneMatrix(entity, record);
        
        m_records[i].push_front(record);
    }
    
    PurgeOldRecords();
}

void LagComp::PurgeOldRecords() {
    float currentTime = sdk::interfaces::g_pGlobalVars->current_time;
    
    for (int i = 0; i < 64; ++i) {
        auto& records = m_records[i];
        while (!records.empty()) {
            auto& front = records.front();
            if (currentTime - front.simulationTime > m_config.maxTime || records.size() > m_config.maxRecords) {
                records.pop_front();
            } else break;
        }
    }
}

bool LagComp::IsRecordValid(const LagRecord& record, sdk::CBaseEntity* entity) {
    if (!record.valid) return false;
    if (record.simulationTime <= 0.0f) return false;
    
    float currentTime = sdk::interfaces::g_pGlobalVars->current_time;
    if (currentTime - record.simulationTime > m_config.maxTime) return false;
    
    return true;
}

float LagComp::GetLerpTime() {
    return 0.1f;
}

void LagComp::StoreBoneMatrix(sdk::CBaseEntity* entity, LagRecord& record) {
}

void LagComp::SetupBones(sdk::CBaseEntity* entity, LagRecord& record) {
}

} // namespace resolver