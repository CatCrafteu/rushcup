#pragma once

#include <vector>
#include <array>
#include <deque>
#include <optional>
#include <memory>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "../../core/sdk/interfaces.hpp"
#include "../../core/config/config_manager.hpp"

namespace resolver {

struct ResolverConfig {
    bool enabled = true;
    enum class Mode { Brute, History, ML } mode = Mode::History;
    std::string overrideKeybind;
    bool logMisses = true;
    int updateRate = 1;
    int maxHistory = 128;
    
    struct AnimFixConfig {
        bool enabled = true;
        bool layers = true;
        bool prediction = true;
    } animFix;
};

struct PlayerRecord {
    sdk::CBaseEntity* entity = nullptr;
    
    struct LayerRecord {
        int sequence = 0;
        float cycle = 0.0f;
        float weight = 0.0f;
        float playbackRate = 0.0f;
        int order = 0;
    };
    
    std::array<LayerRecord, 13> layers;
    sdk::QAngle eyeAngles;
    sdk::QAngle absAngles;
    sdk::Vector3D origin;
    sdk::Vector3D velocity;
    sdk::Vector3D mins, maxs;
    float simulationTime = 0.0f;
    float lowerBodyYawTarget = 0.0f;
    float lastUpdateTime = 0.0f;
    int flags = 0;
    int tickCount = 0;
    bool valid = false;
    bool dormant = false;
    bool shot = false;
    int shotsMissed = 0;
    int shotsHit = 0;
    
    // Resolver data
    float resolvedYaw = 0.0f;
    float resolvedPitch = 0.0f;
    enum class ResolveMode { None, Brute, History, LBY, Freestanding, ML } resolveMode = ResolveMode::None;
    float lbyDelta = 0.0f;
    float lastMovingLby = 0.0f;
    float lastMovingTime = 0.0f;
    bool wasMoving = false;
    std::deque<float> yawHistory;
    std::deque<float> lbyHistory;
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
    
    ResolverConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    std::optional<PlayerRecord> GetRecord(int index);
    void ResolvePlayer(sdk::CBaseEntity* entity, PlayerRecord& record);
    float GetResolvedYaw(int index) const;
    float GetResolvedPitch(int index) const;
    
    // Resolver modes
    void BruteForceResolve(PlayerRecord& record);
    void HistoryResolve(PlayerRecord& record);
    void MLResolve(PlayerRecord& record);
    void LBYResolve(PlayerRecord& record);
    void FreestandingResolve(PlayerRecord& record);
    
    // Miss logging
    void LogMiss(int index, float resolvedYaw, float actualYaw);
    void LogHit(int index);
    
    // Utility
    bool IsResolving(int index) const;
    void ResetPlayer(int index);
    void ResetAll();

private:
    Resolver() = default;
    ResolverConfig m_config;
    
    std::array<PlayerRecord, 64> m_records;
    std::mutex m_mutex;
    
    static constexpr const char* CONFIG_KEY = "resolver";
    
    void UpdateRecords(int stage);
    void StoreLayerData(sdk::CBaseEntity* entity, PlayerRecord& record);
    void StorePoseParams(sdk::CBaseEntity* entity, PlayerRecord& record);
    float CalculateLBYDelta(const PlayerRecord& record);
    bool IsEntityMoving(const PlayerRecord& record);
    float GetFreestandingYaw(sdk::CBaseEntity* entity, const PlayerRecord& record);
    float GetAtTargetsYaw(sdk::CBaseEntity* entity);
    void UpdateYawHistory(PlayerRecord& record);
    void UpdateLBYHistory(PlayerRecord& record);
    int GetMissedShots(int index) const;
    
    // Brute force stages
    static constexpr float BRUTE_STAGES[] = {0, 60, -60, 120, -120, 180, 90, -90, 45, -45, 30, -30};
    static constexpr int NUM_BRUTE_STAGES = 12;
};

struct AnimFixConfig {
    bool enabled = true;
    bool layers = true;
    bool prediction = true;
};

struct AnimationLayer {
    int sequence = 0;
    float cycle = 0.0f;
    float weight = 0.0f;
    float playbackRate = 0.0f;
    int order = 0;
};

class AnimFix {
public:
    struct PlayerAnimData {
        std::array<AnimationLayer, 13> serverLayers;
        std::array<AnimationLayer, 13> clientLayers;
        std::array<float, 24> poseParams;
        sdk::QAngle eyeAngles;
        sdk::QAngle absAngles;
        sdk::Vector3D origin;
        sdk::Vector3D velocity;
        float simulationTime = 0.0f;
        float lowerBodyYawTarget = 0.0f;
        float duckAmount = 0.0f;
        float duckSpeed = 0.0f;
        int flags = 0;
        bool valid = false;
        bool dormant = false;
        int tickCount = 0;
    };
    
    static AnimFix& Instance() {
        static AnimFix instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnFrameStageNotify(int stage);
    
    AnimFixConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    std::optional<PlayerAnimData> GetAnimData(int index);
    void FixAnimation(sdk::CBaseEntity* entity);
    void PredictAnimation(sdk::CBaseEntity* entity, PlayerAnimData& data);
    void UpdateLayers(sdk::CBaseEntity* entity, PlayerAnimData& data);
    void UpdatePoseParams(sdk::CBaseEntity* entity, PlayerAnimData& data);

private:
    AnimFix() = default;
    AnimFixConfig m_config;
    
    std::array<PlayerAnimData, 64> m_animData;
    std::mutex m_mutex;
    
    static constexpr const char* CONFIG_KEY = "resolver.animfix";
    
    void StoreAnimData(sdk::CBaseEntity* entity, PlayerAnimData& data);
    void ApplyAnimFix(sdk::CBaseEntity* entity, PlayerAnimData& data);
    void FixLayerWeights(sdk::CBaseEntity* entity, PlayerAnimData& data);
    void FixLayerCycles(sdk::CBaseEntity* entity, PlayerAnimData& data);
    void FixPoseParams(sdk::CBaseEntity* entity, PlayerAnimData& data);
    bool IsLayerValid(const AnimationLayer& layer);
};

struct LagCompConfig {
    bool enabled = true;
    int maxRecords = 64;
    float maxTime = 0.2f; // 200ms
};

struct LagRecord {
    sdk::Vector3D origin;
    sdk::QAngle angles;
    sdk::Vector3D velocity;
    sdk::QAngle eyeAngles;
    std::array<sdk::Matrix3x4, 128> boneMatrix;
    float simulationTime = 0.0f;
    int flags = 0;
    bool valid = false;
    int tickCount = 0;
    bool shot = false;
};

class LagComp {
public:
    static LagComp& Instance() {
        static LagComp instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnCreateMove(sdk::CUserCmd* cmd);
    void OnFrameStageNotify(int stage);
    
    LagCompConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    std::optional<LagRecord> GetBestRecord(sdk::CBaseEntity* entity);
    std::vector<LagRecord> GetRecords(sdk::CBaseEntity* entity);
    void ApplyLagCompensation(sdk::CBaseEntity* entity, const LagRecord& record);
    void RestoreEntity(sdk::CBaseEntity* entity);
    
    // Visualization
    void RenderRecords();

private:
    LagComp() = default;
    LagCompConfig m_config;
    
    std::array<std::deque<LagRecord>, 64> m_records;
    std::mutex m_mutex;
    
    static constexpr const char* CONFIG_KEY = "resolver.lagcomp";
    static constexpr int MAX_RECORDS = 128;
    
    void UpdateRecords(int stage);
    void PurgeOldRecords();
    bool IsRecordValid(const LagRecord& record, sdk::CBaseEntity* entity);
    float GetLerpTime();
    void StoreBoneMatrix(sdk::CBaseEntity* entity, LagRecord& record);
    void SetupBones(sdk::CBaseEntity* entity, LagRecord& record);
};

} // namespace resolver