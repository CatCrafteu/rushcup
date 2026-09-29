#pragma once

#include <vector>
#include <array>
#include <deque>
#include <optional>
#include <string>
#include "../../core/sdk/structs.hpp"
#include "../../core/sdk/interfaces.hpp"
#include "../../core/config/config_manager.hpp"
#include "../../core/logger/logger.hpp"

namespace features::rage {

struct ResolverConfig {
    bool enabled = true;
    bool resolveTeam = false;
    bool resolveAir = true;
    bool resolveMoving = true;
    int maxMisses = 2;
    float maxDelta = 58.0f;
    bool bruteForce = true;
    int bruteForceAttempts = 3;
    bool logMisses = true;
    bool logResolves = false;
};

struct ResolverData {
    sdk::QAngle lastAngle;
    sdk::QAngle resolvedAngle;
    sdk::QAngle bruteAngle;
    float lastLBY = 0.0f;
    float resolvedLBY = 0.0f;
    int shotsMissed = 0;
    int shotsHit = 0;
    int resolveMode = 0;
    bool isResolved = false;
    bool wasDormant = true;
    float lastUpdateTime = 0.0f;
    std::deque<float> lbyHistory;
    std::deque<sdk::QAngle> angleHistory;
};

class Resolver {
public:
    static Resolver& Instance() {
        static Resolver instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnCreateMove(sdk::CUserCmd* cmd);
    void OnFrameStageNotify(int stage);
    void OnFireEvent(void* event);
    
    ResolverData& GetResolverData(int index);
    void ResetResolverData(int index);
    void OnShotFired(int index, const sdk::QAngle& eyeAngles, bool hit);
    void OnPlayerHurt(int attacker, int victim, int hitgroup, int damage);
    
    float GetResolvedYaw(int index);
    float GetResolvedPitch(int index);
    bool IsResolved(int index) const;
    int GetResolveMode(int index) const;
    
    ResolverConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();

private:
    Resolver() = default;
    ResolverConfig m_config;
    
    std::array<ResolverData, 65> m_resolverData;
    std::mutex m_mutex;
    
    static constexpr const char* CONFIG_KEY = "rage_resolver";
    
    void UpdatePlayer(int index, sdk::CBaseEntity* entity);
    void ResolveYaw(int index, sdk::CBaseEntity* entity);
    void ResolvePitch(int index, sdk::CBaseEntity* entity);
    void ResolveLBY(int index, sdk::CBaseEntity* entity);
    void BruteForceYaw(int index, sdk::CBaseEntity* entity);
    void UpdateLBYHistory(int index, sdk::CBaseEntity* entity);
    bool IsLBYUpdated(int index, sdk::CBaseEntity* entity);
    float GetBestYaw(int index, sdk::CBaseEntity* entity);
    float CalculateFreestandingYaw(int index, sdk::CBaseEntity* entity);
    void OnMiss(int index);
    void OnHit(int index);
};

} // namespace features::rage