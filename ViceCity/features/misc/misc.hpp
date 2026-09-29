#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <deque>
#include "../../core/sdk/structs.hpp"
#include "../../core/sdk/interfaces.hpp"
#include "../../core/config/config_manager.hpp"

namespace features::misc {

struct MovementExConfig {
    struct AutoPeekConfig {
        bool enabled = false;
        std::string keybind;
        bool render = true;
    };
    
    struct ThirdpersonConfig {
        bool enabled = false;
        std::string keybind;
        float distance = 150.0f;
        bool collision = true;
    };
    
    struct FOVOverrideConfig {
        bool enabled = false;
        float value = 90.0f;
    };
    
    struct ViewmodelChangerConfig {
        bool enabled = false;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float fov = 68.0f;
    };
    
    struct AspectRatioConfig {
        bool enabled = false;
        float value = 1.33f;
    };
    
    struct RecoilCrosshairConfig {
        bool enabled = true;
        sdk::Color color = {255, 95, 155, 255};
    };
    
    struct PenetrationCrosshairConfig {
        bool enabled = false;
        sdk::Color color = {255, 215, 0, 255};
    };
    
    AutoPeekConfig autoPeek;
    ThirdpersonConfig thirdperson;
    FOVOverrideConfig fovOverride;
    ViewmodelChangerConfig viewmodelChanger;
    AspectRatioConfig aspectRatio;
    RecoilCrosshairConfig recoilCrosshair;
    PenetrationCrosshairConfig penetrationCrosshair;
};

class MovementEx {
public:
    static MovementEx& Instance() {
        static MovementEx instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnCreateMove(sdk::CUserCmd* cmd);
    void OnFrameStageNotify(int stage);
    void Render();
    
    MovementExConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();

private:
    MovementEx() = default;
    MovementExConfig m_config;
    
    sdk::Vector3D m_autoPeekStartPos;
    bool m_wasAutoPeeking = false;
    sdk::QAngle m_thirdpersonAngles;
    float m_thirdpersonDistance = 150.0f;
    
    static constexpr const char* CONFIG_KEY = "misc_movement";
    
    void DoAutoPeek(sdk::CUserCmd* cmd);
    void DoThirdperson();
    void DoFOVOverride();
    void DoViewmodelChanger();
    void DoAspectRatio();
    void RenderAutoPeek();
    void RenderThirdperson();
    bool IsKeyPressed(const std::string& keybind);
    sdk::CBaseEntity* GetLocalPlayer() const;
    bool WorldToScreen(const sdk::Vector3D& world, sdk::Vector2D& screen) const;
};

struct LogsConfig {
    bool hitLogs = true;
    bool damageLogs = true;
    bool purchaseLogs = true;
    bool consoleLogs = false;
    bool filterLocal = true;
    bool filterTeammates = false;
    bool filterEnemies = true;
};

struct LogEntry {
    enum class Type { Hit, Damage, Purchase, Console } type;
    std::string message;
    sdk::Color color = {255, 255, 255, 255};
    float time = 0.0f;
    float duration = 5.0f;
};

class Logs {
public:
    static Logs& Instance() {
        static Logs instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    void OnFireEvent(void* event);
    void OnDispatchSound(void* sound);
    
    LogsConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    void AddLog(LogEntry::Type type, const std::string& msg, const sdk::Color& color = {255, 255, 255, 255});

private:
    Logs() = default;
    LogsConfig m_config;
    
    std::deque<LogEntry> m_logs;
    std::mutex m_mutex;
    static constexpr size_t MAX_LOGS = 100;
    static constexpr const char* CONFIG_KEY = "misc_logs";
    
    void ProcessEvent(void* event);
    std::string GetHitgroupName(int hitgroup);
    std::string GetWeaponName(int itemDefIndex);
    sdk::Color GetLogColor(LogEntry::Type type);
    void RenderLog(const LogEntry& log, float y);
};

struct SkinchangerConfig {
    bool enabled = true;
    bool autoApply = true;
    bool showInInventory = true;
};

class Skinchanger {
public:
    struct WeaponSkin {
        int itemDefinitionIndex = 0;
        int paintKit = 0;
        float wear = 0.001f;
        int seed = 0;
        int statTrak = -1;
        std::string customName;
        std::array<int, 4> stickers = {0, 0, 0, 0};
        std::array<float, 4> stickerWear = {0.0f, 0.0f, 0.0f, 0.0f};
        int charm = 0;
        bool isSouvenir = false;
        int tournamentID = 0;
        int tournamentStage = 0;
        int tournamentTeam1 = 0;
        int tournamentTeam2 = 0;
    };
    
    static Skinchanger& Instance() {
        static Skinchanger instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void OnFrameStageNotify(int stage);
    void OnPostDataUpdate(int updateType);
    
    SkinchangerConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    void ApplySkin(sdk::CBaseWeapon* weapon, const WeaponSkin& skin);
    std::optional<WeaponSkin> GetSkinForWeapon(int itemDefIndex);
    void SetSkinForWeapon(int itemDefIndex, const WeaponSkin& skin);
    void RemoveSkinForWeapon(int itemDefIndex);
    
    // Knife/Glove models
    void ForceKnifeModel(sdk::CBaseEntity* viewModel, int knifeModel);
    void ForceGloveModel(sdk::CBaseEntity* viewModel, int gloveModel);
    
    // StatTrak
    void UpdateStatTrak(sdk::CBaseWeapon* weapon, int kills);

private:
    Skinchanger() = default;
    SkinchangerConfig m_config;
    
    std::unordered_map<int, WeaponSkin> m_skins;
    std::mutex m_mutex;
    
    static constexpr const char* CONFIG_KEY = "misc_skinchanger";
    static constexpr const char* SKINS_KEY = "skinchanger_inventory.items";
    
    void ApplyAllSkins();
    int GetKnifeModel(int itemDefIndex);
    int GetGloveModel(int itemDefIndex);
    void UpdateWeaponNetworkable(sdk::CBaseWeapon* weapon);
};

struct InventoryUIConfig {
    bool showCaseOpening = true;
    bool showStickerTool = true;
    bool showInspectPanel = true;
    bool showPatternSeedBrowser = true;
};

class InventoryUI {
public:
    static InventoryUI& Instance() {
        static InventoryUI instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    
    InventoryUIConfig& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();
    
    // Case opening
    void OpenCase(int caseId, int keyId);
    void RenderCaseOpening();
    
    // Sticker tool
    void RenderStickerTool();
    
    // Inspect panel
    void RenderInspectPanel();
    
    // Pattern seed browser
    void RenderPatternSeedBrowser();

private:
    InventoryUI() = default;
    InventoryUIConfig m_config;
    
    // Case opening state
    bool m_caseOpening = false;
    int m_caseId = 0;
    int m_keyId = 0;
    std::vector<int> m_rollSequence;
    int m_winnerIndex = 0;
    float m_rollProgress = 0.0f;
    float m_rollDuration = 3.5f;
    bool m_rollComplete = false;
    
    // Inspect state
    sdk::CBaseWeapon* m_inspectWeapon = nullptr;
    bool m_showInspect = false;
    
    // Sticker tool state
    int m_selectedWeapon = 0;
    int m_selectedStickerSlot = 0;
    int m_selectedSticker = 0;
    float m_stickerScale = 1.0f;
    float m_stickerRotation = 0.0f;
    sdk::Vector2D m_stickerOffset = {0, 0};
    
    // Pattern browser state
    int m_browsePaintKit = 0;
    int m_browseSeed = 0;
    float m_browseWear = 0.001f;
    
    static constexpr const char* CONFIG_KEY = "misc_inventory_ui";
};

} // namespace features::misc