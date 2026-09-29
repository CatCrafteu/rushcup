#include "core/pch.hpp"
#include "skinchanger_system/inspect_panel.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "core/logger/logger.hpp"
#include "core/config/config_manager.hpp"

namespace skinchanger {

void InspectPanel::Initialize() {
    LOG_INFO(Skinchanger, "InspectPanel initialized");
}

void InspectPanel::Shutdown() {
    LOG_INFO(Skinchanger, "InspectPanel shutdown");
}

void InspectPanel::Render() {
    if (!m_isOpen) return;
    
    ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
    
    if (ImGui::Begin("Inspect Weapon", &m_isOpen, flags)) {
        if (m_currentInspect.itemId != 0 || m_currentInspect.itemDefIndex != 0) {
            RenderInspectUI();
        } else {
            ImGui::Text("No item selected for inspection");
        }
    }
    ImGui::End();
}

void InspectPanel::Update(float dt) {
    if (!m_isOpen) return;
    
    // Auto-rotate
    if (m_currentInspect.autoRotate) {
        m_currentInspect.cameraYaw += m_currentInspect.autoRotateSpeed * dt;
        if (m_currentInspect.cameraYaw > 360.0f) m_currentInspect.cameraYaw -= 360.0f;
    }
    
    HandleCameraInput();
}

void InspectPanel::OpenInspect(uint64_t itemId) {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) {
        LOG_WARN(Skinchanger, "Item not found for inspect: %llu", itemId);
        return;
    }
    
    const auto& item = *itemOpt;
    m_currentInspect.itemId = item.itemId;
    m_currentInspect.itemDefIndex = item.itemDefIndex;
    m_currentInspect.paintKit = item.paintKit;
    m_currentInspect.wear = item.wear;
    m_currentInspect.seed = item.seed;
    m_currentInspect.statTrak = item.statTrak;
    
    strncpy_s(m_currentInspect.customName, item.customName, 31);
    
    for (int i = 0; i < 5; ++i) {
        m_currentInspect.stickers[i].stickerId = item.stickers[i].stickerId;
        m_currentInspect.stickers[i].slot = item.stickers[i].slot;
        m_currentInspect.stickers[i].wear = item.stickers[i].wear;
        m_currentInspect.stickers[i].scale = item.stickers[i].scale;
        m_currentInspect.stickers[i].rotation = item.stickers[i].rotation;
        m_currentInspect.stickers[i].offset = item.stickers[i].offset;
    }
    
    m_currentInspect.charmId = item.charmId;
    m_currentInspect.isSouvenir = item.isSouvenir;
    m_currentInspect.isTournament = item.isTournament;
    m_currentInspect.tournamentId = item.tournamentId;
    m_currentInspect.tournamentStage = item.tournamentStage;
    m_currentInspect.tournamentTeam1 = item.tournamentTeam1;
    m_currentInspect.tournamentTeam2 = item.tournamentTeam2;
    m_currentInspect.rarity = item.rarity;
    
    ResetCamera();
    m_isOpen = true;
    
    LoadWeaponModel(m_currentInspect.itemDefIndex, m_currentInspect.paintKit);
}

void InspectPanel::OpenInspectByDefIndex(int itemDefIndex, int paintKit, float wear, int seed) {
    m_currentInspect.itemId = 0;
    m_currentInspect.itemDefIndex = itemDefIndex;
    m_currentInspect.paintKit = paintKit;
    m_currentInspect.wear = wear;
    m_currentInspect.seed = seed;
    m_currentInspect.statTrak = -1;
    m_currentInspect.customName[0] = '\0';
    
    for (int i = 0; i < 5; ++i) {
        m_currentInspect.stickers[i] = InspectData::StickerInfo{};
    }
    
    m_currentInspect.charmId = 0;
    m_currentInspect.isSouvenir = false;
    m_currentInspect.isTournament = false;
    m_currentInspect.tournamentId = 0;
    m_currentInspect.tournamentStage = 0;
    m_currentInspect.tournamentTeam1 = 0;
    m_currentInspect.tournamentTeam2 = 0;
    m_currentInspect.rarity = 0;
    
    ResetCamera();
    m_isOpen = true;
    
    LoadWeaponModel(itemDefIndex, paintKit);
}

void InspectPanel::CloseInspect() {
    m_isOpen = false;
    m_currentInspect = InspectData{};
}

void InspectPanel::RenderInspectUI() {
    ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
    
    // Left panel: 3D view
    ImGui::BeginChild("##3dview", ImVec2(500, 0), true);
    Render3DModel();
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // Right panel: Info
    ImGui::BeginChild("##info", ImVec2(0, 0), true);
    RenderUI();
    ImGui::EndChild();
}

void InspectPanel::Render3DModel() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetContentRegionAvail();
    
    // Background
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), m_bgColor1, 8.0f);
    
    // Draw gradient background
    ViceTheme::Instance().DrawGradientRect(drawList, pos, ImVec2(pos.x + size.x, pos.y + size.y),
                                          m_bgColor1, m_bgColor2, true);
    
    // Draw weapon model placeholder
    ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
    float modelScale = std::min(size.x, size.y) * 0.4f;
    
    // Simple weapon silhouette
    ImU32 weaponColor = IM_COL32(100, 100, 120, 255);
    if (m_currentInspect.rarity > 0) {
        // Color based on rarity
        auto& rarityColor = RarityGlow::RARITY_COLORS[m_currentInspect.rarity];
        weaponColor = rarityColor.borderColor;
    }
    
    // Draw weapon outline
    float gunLength = modelScale * 1.5f;
    float gunHeight = modelScale * 0.3f;
    
    drawList->AddRectFilled(
        ImVec2(center.x - gunLength * 0.5f, center.y - gunHeight * 0.5f),
        ImVec2(center.x + gunLength * 0.5f, center.y + gunHeight * 0.5f),
        weaponColor, 8.0f
    );
    
    // Draw stickers on weapon
    RenderStickerOnModel(m_currentInspect.stickers[0], 0);
    RenderStickerOnModel(m_currentInspect.stickers[1], 1);
    RenderStickerOnModel(m_currentInspect.stickers[2], 2);
    RenderStickerOnModel(m_currentInspect.stickers[3], 3);
    
    // Camera controls hint
    drawList->AddText(
        ImVec2(pos.x + 10, pos.y + size.y - 30),
        IM_COL32(180, 165, 175, 255),
        "LMB: Rotate | RMB: Pan | Scroll: Zoom | Double-click: Reset"
    );
    
    ImGui::Dummy(size);
}

void InspectPanel::RenderUI() {
    // Item name and rarity
    std::string itemName = GetWeaponName(m_currentInspect.itemDefIndex);
    if (m_currentInspect.paintKit > 0) {
        itemName += " | " + GetSkinName(m_currentInspect.paintKit);
    }
    if (!m_currentInspect.customName[0] == '\0') {
        itemName = m_currentInspect.customName;
    }
    
    ImGui::Text("%s", itemName.c_str());
    
    // Rarity badge
    if (m_currentInspect.rarity > 0) {
        ImGui::SameLine();
        auto& rarityColor = RarityGlow::RARITY_COLORS[m_currentInspect.rarity];
        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(rarityColor.nameColor));
        ImGui::Text("[%s]", RarityGlow::RARITY_COLORS[m_currentInspect.rarity].name);
        ImGui::PopStyleColor();
    }
    
    ImGui::Separator();
    
    // Wear bar
    RenderWearBar();
    
    // StatTrak
    RenderStatTrak();
    
    // Pattern seed
    RenderPatternSeed();
    
    ImGui::Separator();
    
    // Stickers
    RenderStickerCloseups();
    
    ImGui::Separator();
    
    // Tournament/Souvenir info
    if (m_currentInspect.isSouvenir || m_currentInspect.isTournament) {
        ImGui::Text("Tournament Item");
        if (m_currentInspect.isTournament) {
            ImGui::Text("Tournament: %d", m_currentInspect.tournamentId);
            ImGui::Text("Stage: %d", m_currentInspect.tournamentStage);
            ImGui::Text("Team 1: %d vs Team 2: %d", m_currentInspect.tournamentTeam1, m_currentInspect.tournamentTeam2);
        }
        if (m_currentInspect.isSouvenir) {
            ImGui::Text("Souvenir Item");
        }
        ImGui::Separator();
    }
    
    // Camera controls
    ImGui::Text("Camera Controls");
    ImGui::SliderFloat("Distance", &m_currentInspect.cameraDistance, 50.0f, 500.0f);
    ImGui::SliderFloat("Yaw", &m_currentInspect.cameraYaw, 0.0f, 360.0f);
    ImGui::SliderFloat("Pitch", &m_currentInspect.cameraPitch, -89.0f, 89.0f);
    ImGui::Checkbox("Auto Rotate", &m_currentInspect.autoRotate);
    if (m_currentInspect.autoRotate) {
        ImGui::SliderFloat("Speed", &m_currentInspect.autoRotateSpeed, 1.0f, 100.0f);
    }
    if (ImGui::Button("Reset Camera")) {
        ResetCamera();
    }
}

void InspectPanel::RenderWearBar() {
    float wear = m_currentInspect.wear;
    std::string wearName = GetWearName(wear);
    ImU32 wearColor = GetWearColor(wear);
    
    ImGui::Text("Wear: %s (%.4f)", wearName.c_str(), wear);
    
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    float height = 8.0f;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Background
    drawList->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), IM_COL32(36, 30, 36, 255), 4.0f);
    
    // Wear progress (lower wear = better condition = more to the right)
    float progress = 1.0f - wear;
    ImVec2 fillEnd = ImVec2(pos.x + width * progress, pos.y + height);
    
    // Gradient from red to green based on wear
    ImU32 color = wearColor;
    drawList->AddRectFilled(pos, fillEnd, color, 4.0f);
    
    // Wear tier markers
    const float tierBoundaries[] = {0.07f, 0.15f, 0.38f, 0.45f};
    const char* tierNames[] = {"FN", "MW", "FT", "WW", "BS"};
    
    for (int i = 0; i < 4; ++i) {
        float x = pos.x + width * (1.0f - tierBoundaries[i]);
        drawList->AddLine(
            ImVec2(x, pos.y),
            ImVec2(x, pos.y + height),
            IM_COL32(255, 255, 255, 100)
        );
    }
    
    // Tier labels
    for (int i = 0; i < 5; ++i) {
        float t = static_cast<float>(i) / 4.0f;
        float x = pos.x + width * t - 15;
        drawList->AddText(ImVec2(x, pos.y + height + 2), IM_COL32(180, 165, 175, 255), tierNames[i]);
    }
    
    ImGui::Dummy(ImVec2(0, height + 20));
}

void InspectPanel::RenderStatTrak() {
    if (m_currentInspect.statTrak >= 0) {
        ImGui::Text("StatTrak: %d kills", m_currentInspect.statTrak);
        
        // StatTrak display
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        std::string statText = fmt::format("%d", m_currentInspect.statTrak);
        ImVec2 textSize = ImGui::CalcTextSize(statText.c_str());
        
        drawList->AddRectFilled(
            pos, ImVec2(pos.x + textSize.x + 16, pos.y + textSize.y + 8),
            IM_COL32(28, 24, 28, 255), 4.0f
        );
        drawList->AddRect(
            pos, ImVec2(pos.x + textSize.x + 16, pos.y + textSize.y + 8),
            IM_COL32(255, 215, 0, 255), 4.0f
        );
        drawList->AddText(ImVec2(pos.x + 8, pos.y + 4), IM_COL32(255, 215, 0, 255), statText.c_str());
        
        ImGui::Dummy(ImVec2(0, textSize.y + 16));
    }
}

void InspectPanel::RenderPatternSeed() {
    if (m_currentInspect.paintKit > 0) {
        ImGui::Text("Pattern Seed: %d", m_currentInspect.seed);
        
        std::string patternDesc = GetPatternDescription(m_currentInspect.paintKit, m_currentInspect.seed);
        if (!patternDesc.empty()) {
            ImGui::Text("Pattern: %s", patternDesc.c_str());
        }
        
        ImGui::Separator();
        
        // Seed browser
        if (ImGui::Button("Browse Seeds")) {
            // Open pattern seed browser
        }
        ImGui::SameLine();
        if (ImGui::Button("Random Seed")) {
            m_currentInspect.seed = rand() % 1001;
        }
    }
}

void InspectPanel::RenderStickerCloseups() {
    bool hasStickers = false;
    for (int i = 0; i < 5; ++i) {
        if (m_currentInspect.stickers[i].stickerId > 0) {
            hasStickers = true;
            break;
        }
    }
    
    if (!hasStickers) return;
    
    ImGui::Text("Stickers");
    ImGui::Separator();
    
    for (int i = 0; i < 5; ++i) {
        const auto& sticker = m_currentInspect.stickers[i];
        if (sticker.stickerId == 0) continue;
        
        ImGui::PushID(i);
        
        std::string stickerName = GetStickerName(sticker.stickerId);
        ImGui::Text("Slot %d: %s", i + 1, stickerName.c_str());
        
        // Sticker preview
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        float size = 80.0f;
        ImU32 stickerColor = GetStickerColor(sticker.stickerId);
        
        drawList->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), stickerColor, 4.0f);
        drawList->AddRect(pos, ImVec2(pos.x + size, pos.y + size), IM_COL32(255, 215, 0, 255), 4.0f, 0, 2.0f);
        
        // Sticker details
        ImGui::Dummy(ImVec2(0, size + 4));
        ImGui::Text("  Wear: %.4f", sticker.wear);
        ImGui::Text("  Scale: %.2f", sticker.scale);
        ImGui::Text("  Rotation: %.1f", sticker.rotation);
        ImGui::Text("  Offset: (%.1f, %.1f)", sticker.offset.x, sticker.offset.y);
        
        ImGui::PopID();
    }
}

void InspectPanel::HandleCameraInput() {
    if (!ImGui::IsWindowHovered()) return;
    
    ImGuiIO& io = ImGui::GetIO();
    
    // Rotate with left mouse
    if (io.MouseDown[ImGuiMouseButton_Left]) {
        m_currentInspect.cameraYaw += io.MouseDelta.x * 0.5f;
        m_currentInspect.cameraPitch = std::clamp(m_currentInspect.cameraPitch - io.MouseDelta.y * 0.5f, -89.0f, 89.0f);
    }
    
    // Pan with right mouse
    if (io.MouseDown[ImGuiMouseButton_Right]) {
        m_currentInspect.modelOffset.x += io.MouseDelta.x * 0.5f;
        m_currentInspect.modelOffset.y -= io.MouseDelta.y * 0.5f;
    }
    
    // Zoom with scroll
    if (io.MouseWheel != 0.0f) {
        m_currentInspect.cameraDistance = std::clamp(m_currentInspect.cameraDistance - io.MouseWheel * 10.0f, 50.0f, 500.0f);
    }
    
    // Reset on double click
    if (io.MouseDoubleClicked[ImGuiMouseButton_Left]) {
        ResetCamera();
    }
}

void InspectPanel::ResetCamera() {
    m_currentInspect.cameraDistance = 150.0f;
    m_currentInspect.cameraYaw = 0.0f;
    m_currentInspect.cameraPitch = 20.0f;
    m_currentInspect.modelOffset = {0, 0, 0};
}

void InspectPanel::LoadWeaponModel(int itemDefIndex, int paintKit) {
    // Would load actual weapon model
    // For now, just log
    LOG_INFO(Skinchanger, "Loading weapon model: defIndex=%d, paintKit=%d", itemDefIndex, paintKit);
}

void InspectPanel::LoadStickerModels() {
    // Would load sticker models
}

void InspectPanel::RenderStickerOnModel(const InspectData::StickerInfo& sticker, int slot) {
    if (sticker.stickerId == 0) return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Calculate sticker position on weapon
    // This is simplified - real implementation would use weapon UV mapping
    float baseX = 100.0f + slot * 120.0f;
    float baseY = 100.0f;
    
    ImVec2 stickerPos = ImVec2(baseX + sticker.offset.x, baseY + sticker.offset.y);
    float stickerSize = 40.0f * sticker.scale;
    
    ImU32 color = GetStickerColor(sticker.stickerId);
    
    drawList->AddRectFilled(
        stickerPos,
        ImVec2(stickerPos.x + stickerSize, stickerPos.y + stickerSize),
        color, 4.0f
    );
    
    drawList->AddRect(
        stickerPos,
        ImVec2(stickerPos.x + stickerSize, stickerPos.y + stickerSize),
        IM_COL32(255, 215, 0, 255), 4.0f
    );
}

void InspectPanel::CreateRarityGlowEffect(uint32_t rarity) {
    // Create glow effect for rarity
}

ImU32 InspectPanel::GetWearColor(float wear) const {
    if (wear < 0.07f) return IM_COL32(0, 200, 0, 255); // Factory New - Green
    if (wear < 0.15f) return IM_COL32(100, 255, 100, 255); // Minimal Wear - Light Green
    if (wear < 0.38f) return IM_COL32(255, 255, 0, 255); // Field-Tested - Yellow
    if (wear < 0.45f) return IM_COL32(255, 150, 0, 255); // Well-Worn - Orange
    return IM_COL32(255, 50, 50, 255); // Battle-Scarred - Red
}

std::string InspectPanel::GetWearName(float wear) const {
    if (wear < 0.07f) return "Factory New";
    if (wear < 0.15f) return "Minimal Wear";
    if (wear < 0.38f) return "Field-Tested";
    if (wear < 0.45f) return "Well-Worn";
    return "Battle-Scarred";
}

std::string InspectPanel::GetWeaponName(int itemDefIndex) const {
    static const std::unordered_map<int, std::string> weaponNames = {
        {1, "Desert Eagle"}, {2, "Dual Berettas"}, {3, "Five-SeveN"}, {4, "Glock-18"},
        {7, "AK-47"}, {8, "AUG"}, {9, "AWP"}, {10, "FAMAS"}, {11, "G3SG1"},
        {13, "Galil AR"}, {14, "M249"}, {16, "M4A4"}, {17, "MAC-10"}, {19, "P90"},
        {23, "MP5-SD"}, {24, "UMP-45"}, {25, "XM1014"}, {26, "PP-Bizon"}, {27, "MAG-7"},
        {28, "Negev"}, {29, "Sawed-Off"}, {30, "Tec-9"}, {31, "Zeus x27"}, {32, "P2000"},
        {33, "MP7"}, {34, "MP9"}, {35, "Nova"}, {36, "P250"}, {38, "SCAR-20"},
        {39, "SG 553"}, {40, "SSG 08"}, {42, "Knife"}, {43, "Flashbang"}, {44, "HE Grenade"},
        {45, "Smoke Grenade"}, {46, "Molotov"}, {47, "Decoy"}, {48, "Incendiary"},
        {49, "C4"}, {57, "Health Shot"}, {59, "Knife (T)"}, {60, "M4A1-S"}, {61, "USP-S"},
        {63, "CZ75-Auto"}, {64, "R8 Revolver"}, {500, "Bayonet"}, {503, "Classic Knife"},
        {505, "Flip Knife"}, {506, "Gut Knife"}, {507, "Karambit"}, {508, "M9 Bayonet"},
        {509, "Huntsman Knife"}, {512, "Falchion Knife"}, {514, "Bowie Knife"},
        {515, "Butterfly Knife"}, {516, "Shadow Daggers"}, {517, "Paracord Knife"},
        {518, "Survival Knife"}, {519, "Ursus Knife"}, {520, "Navaja Knife"},
        {521, "Nomad Knife"}, {522, "Stiletto Knife"}, {523, "Talon Knife"},
        {525, "Skeleton Knife"}, {5027, "Bloodhound Gloves"}, {5028, "T-Side Gloves"},
        {5029, "CT-Side Gloves"}, {5030, "Sporty Gloves"}, {5031, "Slick Gloves"},
        {5032, "Leather Gloves"}, {5033, "Motorcycle Gloves"}, {5034, "Specialist Gloves"},
        {5035, "Hydra Gloves"}
    };
    
    auto it = weaponNames.find(itemDefIndex);
    if (it != weaponNames.end()) return it->second;
    return fmt::format("Weapon {}", itemDefIndex);
}

std::string InspectPanel::GetSkinName(int paintKit) const {
    // Would look up paint kit name
    return fmt::format("Skin #{}", paintKit);
}

std::string InspectPanel::GetStickerName(int stickerId) const {
    static const std::unordered_map<int, std::string> stickerNames = {
        {1, "Team Dignitas (Holo) | Katowice 2014"},
        {2, "Reason Gaming (Holo) | Katowice 2014"},
        {3, "iBUYPOWER (Holo) | Katowice 2014"},
        {4, "Titan (Holo) | Katowice 2014"},
        {5, "ESL One Cologne 2014"},
        {6, "DreamHack 2014"},
    };
    
    auto it = stickerNames.find(stickerId);
    if (it != stickerNames.end()) return it->second;
    return fmt::format("Sticker {}", stickerId);
}

ImU32 InspectPanel::GetStickerColor(int stickerId) const {
    // Return color based on sticker
    return IM_COL32(255, 215, 0, 255); // Gold for holo
}

std::string InspectPanel::GetPatternDescription(int paintKit, int seed) const {
    // Would look up pattern description
    if (seed == 0) return "Default pattern";
    return fmt::format("Pattern variation #{}", seed % 100);
}

sdk::Vector2D InspectPanel::GetPatternOffset(int paintKit, int seed) const {
    // Calculate pattern offset based on seed
    float x = static_cast<float>(seed % 100) / 100.0f;
    float y = static_cast<float>(seed / 100) / 10.0f;
    return {x, y};
}

} // namespace skinchanger