#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "inventory_core.hpp"

namespace skinchanger {

// Inspect panel (in-game F key inspect - 3D model viewer)
class InspectPanel {
public:
    struct InspectData {
        uint64_t itemId = 0;
        int itemDefIndex = 0;
        int paintKit = 0;
        float wear = 0.0f;
        int seed = 0;
        int statTrak = -1;
        std::string customName;
        struct StickerInfo {
            int stickerId = 0;
            int slot = 0;
            float wear = 0.0f;
            float scale = 1.0f;
            float rotation = 0.0f;
            sdk::Vector2D offset = {0, 0};
        };
        std::array<StickerInfo, 5> stickers;
        int charmId = 0;
        bool isSouvenir = false;
        bool isTournament = false;
        int tournamentId = 0;
        int tournamentStage = 0;
        int tournamentTeam1 = 0;
        int tournamentTeam2 = 0;
        uint32_t rarity = 0;
        // 3D view state
        float cameraDistance = 150.0f;
        float cameraYaw = 0.0f;
        float cameraPitch = 20.0f;
        sdk::Vector3D modelOffset = {0, 0, 0};
        bool autoRotate = true;
        float autoRotateSpeed = 10.0f;
    };
    
    static InspectPanel& Instance() {
        static InspectPanel instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    void Update(float dt);
    
    // Open inspect for item
    void OpenInspect(uint64_t itemId);
    void OpenInspectByDefIndex(int itemDefIndex, int paintKit = 0, float wear = 0.001f, int seed = 0);
    void CloseInspect();
    
    // State
    bool IsOpen() const { return m_isOpen; }
    InspectData& GetCurrentInspect() { return m_currentInspect; }
    const InspectData& GetCurrentInspect() const { return m_currentInspect; }
    
    // 3D rendering
    void Render3DModel();
    void RenderUI();
    void RenderWearBar();
    void RenderStickerCloseups();
    void RenderPatternSeed();
    void RenderStatTrak();
    void RenderRarityGlow();
    
    // Camera controls
    void HandleCameraInput();
    void ResetCamera();
    
    // Lighting
    struct LightingConfig {
        sdk::Vector3D lightDir = {0.5f, -1.0f, 0.5f};
        sdk::Color ambientColor = {60, 60, 70, 255};
        sdk::Color diffuseColor = {255, 255, 255, 255};
        sdk::Color specularColor = {255, 255, 255, 255};
        float ambientIntensity = 0.3f;
        float diffuseIntensity = 1.0f;
        float specularIntensity = 0.5f;
        float specularPower = 32.0f;
    };
    
    LightingConfig& GetLighting() { return m_lighting; }
    
    // Background
    enum class BackgroundType { Gradient, Solid, Image, Cubemap };
    BackgroundType m_backgroundType = BackgroundType::Gradient;
    sdk::Color m_bgColor1 = {28, 24, 28, 255};
    sdk::Color m_bgColor2 = {36, 30, 36, 255};
    std::string m_bgImagePath;
    std::string m_cubemapPath;

private:
    InspectPanel() = default;
    bool m_isOpen = false;
    InspectData m_currentInspect;
    LightingConfig m_lighting;
    std::mutex m_mutex;
    
    // Model rendering
    void* m_modelHandle = nullptr;
    std::vector<uint8_t> m_modelVertices;
    std::vector<uint32_t> m_modelIndices;
    void* m_vertexBuffer = nullptr;
    void* m_indexBuffer = nullptr;
    void* m_textureSRV = nullptr;
    
    void LoadWeaponModel(int itemDefIndex, int paintKit);
    void LoadStickerModels();
    void RenderStickerOnModel(const InspectData::StickerInfo& sticker, int slot);
    void CreateRarityGlowEffect(uint32_t rarity);
    
    // Wear visualization
    sdk::Color GetWearColor(float wear) const;
    std::string GetWearName(float wear) const;
    
    // Pattern seed display
    std::string GetPatternDescription(int paintKit, int seed) const;
    sdk::Vector2D GetPatternOffset(int paintKit, int seed) const;
};

} // namespace skinchanger