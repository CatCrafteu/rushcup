#pragma once

#include <vector>
#include <optional>
#include <array>
#include <string>
#include <unordered_map>
#include "../../core/sdk/structs.hpp"
#include "../../core/sdk/interfaces.hpp"
#include "../../core/config/config_manager.hpp"

namespace features::visuals {

struct ESPBoxConfig {
    bool enabled = true;
    enum class Type { Box2D, Box3D, Corner } type = Type::Corner;
    sdk::Color color = {255, 95, 155, 255};
    sdk::Color colorTeammate = {95, 155, 255, 255};
};

struct ESPSkeletonConfig {
    bool enabled = true;
    sdk::Color color = {255, 215, 0, 255};
};

struct ESPHealthBarConfig {
    bool enabled = true;
    sdk::Color color = {0, 255, 0, 255};
    sdk::Color colorLow = {255, 0, 0, 255};
};

struct ESPHealthTextConfig {
    bool enabled = true;
    sdk::Color color = {255, 255, 255, 255};
};

struct ESPArmorConfig {
    bool enabled = true;
    sdk::Color color = {0, 150, 255, 255};
};

struct ESPAmmoConfig {
    bool enabled = true;
    sdk::Color color = {255, 200, 0, 255};
};

struct ESPNameConfig {
    bool enabled = true;
    sdk::Color color = {255, 255, 255, 255};
};

struct ESPWeaponConfig {
    bool enabled = true;
    enum class Type { Text, Icon, Ammo } type = Type::Text;
    sdk::Color color = {200, 200, 200, 255};
};

struct ESPFlagsConfig {
    bool enabled = true;
    bool showMoney = true;
    bool showArmor = true;
    bool showKit = true;
    bool showScoped = true;
    bool showFlashed = true;
    bool showDefusing = true;
    sdk::Color color = {255, 255, 255, 255};
};

struct ESPDormantConfig {
    bool enabled = true;
    sdk::Color color = {100, 100, 100, 150};
};

struct ESPSnaplinesConfig {
    bool enabled = false;
    enum class Type { Bottom, Center, Top } type = Type::Bottom;
    sdk::Color color = {255, 95, 155, 255};
};

struct ESPOffscreenArrowsConfig {
    bool enabled = true;
    float size = 12.0f;
    float distance = 200.0f;
    sdk::Color color = {255, 95, 155, 255};
};

struct ESPConfig {
    bool enabled = true;
    ESPBoxConfig box;
    ESPSkeletonConfig skeleton;
    ESPHealthBarConfig healthBar;
    ESPHealthTextConfig healthText;
    ESPArmorConfig armor;
    ESPAmmoConfig ammo;
    ESPNameConfig name;
    ESPWeaponConfig weapon;
    ESPFlagsConfig flags;
    ESPDormantConfig dormant;
    ESPSnaplinesConfig snaplines;
    ESPOffscreenArrowsConfig offscreenArrows;
};

class ESP {
public:
    struct PlayerESPData {
        sdk::CBaseEntity* entity = nullptr;
        sdk::Vector2D screenPos;
        sdk::Vector2D screenPosHead;
        sdk::Vector2D screenPosFoot;
        float boxWidth = 0;
        float boxHeight = 0;
        float boxX = 0;
        float boxY = 0;
        int health = 0;
        int armor = 0;
        int maxHealth = 100;
        std::string name;
        std::string weaponName;
        int ammo = 0;
        int maxAmmo = 0;
        bool isDormant = false;
        bool isTeammate = false;
        bool isLocalPlayer = false;
        bool hasDefuser = false;
        bool isScoped = false;
        bool isFlashed = false;
        bool isDefusing = false;
        int money = 0;
        sdk::Vector3D origin;
        sdk::QAngle angles;
        std::array<sdk::Matrix3x4, 128> boneMatrix;
        bool hasBoneMatrix = false;
        std::vector<std::string> flags;
    };
    
    static ESP& Instance() {
        static ESP instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    void OnFrameStageNotify(int stage);
    
    ESPConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    std::vector<PlayerESPData> GetPlayers();

private:
    ESP() = default;
    ESPConfig m_config;
    
    std::vector<PlayerESPData> m_players;
    std::mutex m_mutex;
    
    static constexpr const char* CONFIG_KEY = "visuals_esp";
    
    void UpdatePlayers();
    void CollectPlayerData(sdk::CBaseEntity* entity, PlayerESPData& data);
    bool WorldToScreen(const sdk::Vector3D& world, sdk::Vector2D& screen);
    sdk::CBaseEntity* GetLocalPlayer() const;
    sdk::CBaseWeapon* GetActiveWeapon() const;
    void CalculateBox(const PlayerESPData& data);
    void RenderPlayer(PlayerESPData& data);
    
    // Render helpers
    void DrawBox(const PlayerESPData& data);
    void DrawCornerBox(const PlayerESPData& data);
    void Draw3DBox(const PlayerESPData& data);
    void DrawSkeleton(const PlayerESPData& data);
    void DrawHealthBar(const PlayerESPData& data);
    void DrawHealthText(const PlayerESPData& data);
    void DrawArmor(const PlayerESPData& data);
    void DrawAmmo(const PlayerESPData& data);
    void DrawName(const PlayerESPData& data);
    void DrawWeapon(const PlayerESPData& data);
    void DrawFlags(const PlayerESPData& data);
    void DrawSnapline(const PlayerESPData& data);
    void DrawOffscreenArrow(const PlayerESPData& data);
    void DrawDormant(const PlayerESPData& data);
    
    sdk::Color GetPlayerColor(const PlayerESPData& data, const sdk::Color& enemyColor, const sdk::Color& teamColor) const;
    sdk::Color LerpColor(const sdk::Color& a, const sdk::Color& b, float t) const;
    std::string GetWeaponName(sdk::CBaseWeapon* weapon);
    std::string GetWeaponIcon(sdk::CBaseWeapon* weapon);
};

struct ChamsConfig {
    bool enabled = true;
    
    struct ChamsMaterialConfig {
        bool enabled = true;
        enum class Material { Flat, Glass, Plastic, Metallic, Glow, Wireframe, Pulse } material = Material::Flat;
        sdk::Color color = {255, 95, 155, 255};
        sdk::Color colorTeammate = {95, 155, 255, 255};
    };
    
    struct PlayerChamsConfig {
        ChamsMaterialConfig visible;
        ChamsMaterialConfig invisible;
        ChamsMaterialConfig history;
        ChamsMaterialConfig shot;
        float historyDuration = 1.0f;
        float shotDuration = 0.5f;
    };
    
    PlayerChamsConfig player;
    ChamsMaterialConfig arms;
    ChamsMaterialConfig weapons;
    ChamsMaterialConfig attachments;
};

class Chams {
public:
    struct ChamsContext {
        sdk::CBaseEntity* entity = nullptr;
        int materialType = 0; // 0=visible, 1=invisible, 2=history, 3=shot
        sdk::Color color;
        bool isTeammate = false;
    };
    
    static Chams& Instance() {
        static Chams instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnDrawModelExecute(void* ctx, void* state, const void* info, const sdk::Matrix3x4* boneToWorld);
    void OnFrameStageNotify(int stage);
    
    ChamsConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    void ApplyChams(const ChamsContext& ctx);
    void CreateMaterials();
    void DestroyMaterials();

private:
    Chams() = default;
    ChamsConfig m_config;
    
    // Materials
    sdk::IMaterial* m_materialFlat = nullptr;
    sdk::IMaterial* m_materialFlatIgnoreZ = nullptr;
    sdk::IMaterial* m_materialGlass = nullptr;
    sdk::IMaterial* m_materialGlassIgnoreZ = nullptr;
    sdk::IMaterial* m_materialPlastic = nullptr;
    sdk::IMaterial* m_materialPlasticIgnoreZ = nullptr;
    sdk::IMaterial* m_materialMetallic = nullptr;
    sdk::IMaterial* m_materialMetallicIgnoreZ = nullptr;
    sdk::IMaterial* m_materialGlow = nullptr;
    sdk::IMaterial* m_materialGlowIgnoreZ = nullptr;
    sdk::IMaterial* m_materialWireframe = nullptr;
    sdk::IMaterial* m_materialWireframeIgnoreZ = nullptr;
    sdk::IMaterial* m_materialPulse = nullptr;
    sdk::IMaterial* m_materialPulseIgnoreZ = nullptr;
    
    std::mutex m_mutex;
    float m_pulseTime = 0.0f;
    
    static constexpr const char* CONFIG_KEY = "visuals_chams";
    
    sdk::IMaterial* GetMaterial(ChamsConfig::ChamsMaterialConfig::Material type, bool ignoreZ);
    void ApplyMaterial(sdk::IMaterial* mat, const sdk::Color& color, bool ignoreZ);
    void SetupPulseMaterial(float time);
};

struct WorldConfig {
    bool enabled = true;
    struct NightmodeConfig {
        bool enabled = false;
        sdk::Color color = {20, 20, 30, 255};
    };
    
    struct PropTransparencyConfig {
        bool enabled = false;
        float amount = 0.5f;
    };
    
    struct GrenadePreviewConfig {
        bool enabled = true;
        bool trajectory = true;
        bool bounce = true;
        sdk::Color color = {255, 215, 0, 255};
    };
    
    struct HitmarkerConfig {
        bool enabled = true;
        enum class Type { Type2D, Type3D, Sound } type = Type::Type3D;
        sdk::Color color = {255, 255, 255, 255};
        float duration = 0.5f;
    };
    
    struct DamageIndicatorConfig {
        bool enabled = true;
        sdk::Color color = {255, 100, 100, 255};
    };
    
    struct SpreadCircleConfig {
        bool enabled = true;
        sdk::Color color = {255, 95, 155, 150};
    };
    
    NightmodeConfig nightmode;
    PropTransparencyConfig propTransparency;
    GrenadePreviewConfig grenadePreview;
    HitmarkerConfig hitmarker;
    DamageIndicatorConfig damageIndicator;
    SpreadCircleConfig spreadCircle;
};

class World {
public:
    struct HitmarkerData {
        sdk::Vector3D position;
        float time = 0.0f;
        int damage = 0;
        bool isHeadshot = false;
    };
    
    struct GrenadeTrajectory {
        std::vector<sdk::Vector3D> points;
        sdk::Color color;
        float time = 0.0f;
    };
    
    static World& Instance() {
        static World instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    void OnFrameStageNotify(int stage);
    void OnFireEvent(void* event);
    void OnDispatchSound(void* sound);
    
    WorldConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    void AddHitmarker(const sdk::Vector3D& pos, int damage, bool headshot);
    void AddGrenadeTrajectory(const std::vector<sdk::Vector3D>& points);
    sdk::CBaseEntity* GetLocalPlayer() const;
    sdk::CBaseWeapon* GetActiveWeapon() const;

private:
    World() = default;
    WorldConfig m_config;
    
    std::vector<HitmarkerData> m_hitmarkers;
    std::vector<GrenadeTrajectory> m_grenadeTrajectories;
    std::mutex m_mutex;
    
    std::vector<sdk::IMaterial*> m_nightmodeMaterials;
    bool m_nightmodeApplied = false;
    
    static constexpr const char* CONFIG_KEY = "visuals_world";
    
    void ApplyNightmode();
    void RemoveNightmode();
    void UpdatePropTransparency();
    void RenderHitmarkers();
    void RenderGrenadePreview();
    void RenderDamageIndicators();
    void RenderSpreadCircle();
    void SimulateGrenadeTrajectory(sdk::CBaseEntity* grenade, std::vector<sdk::Vector3D>& points);
};

struct RadarConfig {
    bool enabled = true;
    float range = 3000.0f;
    float size = 200.0f;
    sdk::Vector2D position = {50.0f, 50.0f};
    bool showTeammates = true;
    bool showEnemies = true;
    bool showBomb = true;
    bool customIcons = false;
};

class Radar {
public:
    struct RadarPlayer {
        sdk::Vector2D position;
        bool isTeammate = false;
        bool isLocalPlayer = false;
        int health = 0;
        bool isDormant = false;
    };
    
    static Radar& Instance() {
        static Radar instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
    RadarConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();

private:
    Radar() = default;
    RadarConfig m_config;
    
    std::vector<RadarPlayer> m_players;
    std::mutex m_mutex;
    
    static constexpr const char* CONFIG_KEY = "visuals_radar";
    
    void UpdatePlayers();
    void RenderBackground();
    void RenderPlayers();
    void RenderBomb();
    sdk::Vector2D WorldToRadar(const sdk::Vector3D& worldPos);
    bool IsInRadarRange(const sdk::Vector3D& worldPos);
    sdk::CBaseEntity* GetLocalPlayer() const;
    sdk::CBaseWeapon* GetActiveWeapon() const;
};

struct EffectsConfig {
    struct BulletTracersConfig {
        bool enabled = true;
        enum class Type { Line, Beam, Particle } type = Type::Beam;
        sdk::Color color = {255, 95, 155, 255};
        float duration = 2.0f;
    };
    
    struct ImpactBeamsConfig {
        bool enabled = true;
        sdk::Color color = {255, 215, 0, 255};
        float duration = 3.0f;
    };
    
    struct BulletBeamsConfig {
        bool enabled = false;
        sdk::Color color = {255, 95, 155, 200};
    };
    
    struct HitNumbersConfig {
        bool enabled = true;
        sdk::Color color = {255, 255, 255, 255};
        float fontSize = 14.0f;
    };
    
    BulletTracersConfig bulletTracers;
    ImpactBeamsConfig impactBeams;
    BulletBeamsConfig bulletBeams;
    HitNumbersConfig hitNumbers;
};

class Effects {
public:
    struct TracerData {
        sdk::Vector3D start;
        sdk::Vector3D end;
        sdk::Color color;
        float time = 0.0f;
        float duration = 2.0f;
    };
    
    struct ImpactData {
        sdk::Vector3D position;
        sdk::Vector3D normal;
        sdk::Color color;
        float time = 0.0f;
        float duration = 3.0f;
    };
    
    struct HitNumberData {
        sdk::Vector3D position;
        int damage;
        bool headshot;
        float time = 0.0f;
        float duration = 1.5f;
    };
    
    static Effects& Instance() {
        static Effects instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    void OnFireEvent(void* event);
    
    EffectsConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    void AddTracer(const sdk::Vector3D& start, const sdk::Vector3D& end, const sdk::Color& color);
    void AddImpact(const sdk::Vector3D& pos, const sdk::Vector3D& normal, const sdk::Color& color);
    void AddHitNumber(const sdk::Vector3D& pos, int damage, bool headshot);
    sdk::CBaseEntity* GetLocalPlayer() const;
    sdk::CBaseWeapon* GetActiveWeapon() const;

private:
    Effects() = default;
    EffectsConfig m_config;
    
    std::vector<TracerData> m_tracers;
    std::vector<ImpactData> m_impacts;
    std::vector<HitNumberData> m_hitNumbers;
    std::mutex m_mutex;
    
    static constexpr const char* CONFIG_KEY = "visuals_effects";
    
    void RenderTracers();
    void RenderImpacts();
    void RenderHitNumbers();
    void DrawBeam(const sdk::Vector3D& start, const sdk::Vector3D& end, const sdk::Color& color, float width = 2.0f);
    void DrawBeamRing(const sdk::Vector3D& pos, const sdk::Vector3D& normal, const sdk::Color& color, float radius, float width);
    void DrawBeam(const ImVec2& start, const ImVec2& end, const sdk::Color& color, float width);
    void DrawBeamRing(const ImVec2& pos, const ImVec2& normal, const sdk::Color& color, float radius, float width);
};

} // namespace features::visuals