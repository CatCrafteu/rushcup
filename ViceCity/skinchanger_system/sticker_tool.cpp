#include "core/pch.hpp"
#include "skinchanger_system/sticker_tool.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "core/logger/logger.hpp"
#include "core/config/config_manager.hpp"

namespace skinchanger {

void StickerTool::Initialize() {
    InitializeStickers();
    LoadPresets();
    InitializeStickerPositions();
    LOG_INFO(Skinchanger, "StickerTool initialized");
}

void StickerTool::Shutdown() {
    SavePresets();
    LOG_INFO(Skinchanger, "StickerTool shutdown");
}

void StickerTool::Render() {
    if (!m_uiState.showPresets && m_uiState.selectedItemId == 0) return;
    
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Sticker Tool", &m_uiState.showPresets)) {
        // Item selection
        if (m_uiState.selectedItemId == 0) {
            ImGui::Text("Select a weapon to apply stickers");
            ImGui::Separator();
            
            auto& inv = InventoryCore::Instance();
            auto weapons = inv.GetWeapons();
            
            for (const auto& weapon : weapons) {
                std::string name = fmt::format("{} (ID: {})", GetWeaponName(weapon.itemDefIndex), weapon.itemId);
                if (ImGui::Selectable(name.c_str(), false)) {
                    m_uiState.selectedItemId = weapon.itemId;
                }
            }
        } else {
            // Sticker tool UI
            RenderStickerToolUI();
        }
    }
    ImGui::End();
}

void StickerTool::RenderStickerToolUI() {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(m_uiState.selectedItemId);
    
    if (!itemOpt) {
        m_uiState.selectedItemId = 0;
        return;
    }
    
    const auto& item = *itemOpt;
    int maxSlots = GetMaxSlotsForWeapon(item.itemDefIndex);
    
    ImGui::Text("Weapon: %s", GetWeaponName(item.itemDefIndex).c_str());
    ImGui::SameLine();
    if (ImGui::Button("Back")) {
        m_uiState.selectedItemId = 0;
        return;
    }
    ImGui::Separator();
    
    // Sticker slots
    for (int slot = 0; slot < maxSlots; ++slot) {
        ImGui::PushID(slot);
        
        ImGui::Text("Slot %d", slot + 1);
        ImGui::SameLine();
        
        // Current sticker
        int currentSticker = (slot < 5) ? item.stickers[slot].stickerId : 0;
        std::string stickerName = currentSticker > 0 ? GetStickerName(currentSticker) : "Empty";
        
        if (ImGui::BeginCombo("##sticker", stickerName.c_str())) {
            // Search
            ImGui::InputText("Search", &m_uiState.searchQuery);
            ImGui::Separator();
            
            // Rarity filter
            if (ImGui::BeginCombo("Rarity", m_uiState.selectedRarity >= 0 ? RARITY_NAMES[m_uiState.selectedRarity] : "All")) {
                if (ImGui::Selectable("All", m_uiState.selectedRarity == -1)) m_uiState.selectedRarity = -1;
                for (int r = 0; r < 8; ++r) {
                    if (ImGui::Selectable(RARITY_NAMES[r], m_uiState.selectedRarity == r)) m_uiState.selectedRarity = r;
                }
                ImGui::EndCombo();
            }
            
            // Owned only
            ImGui::Checkbox("Owned Only", &m_uiState.showOnlyOwned);
            ImGui::Separator();
            
            // Sticker list
            auto stickers = GetAllStickers();
            for (const auto& sticker : stickers) {
                if (m_uiState.selectedRarity >= 0 && sticker.rarity != m_uiState.selectedRarity) continue;
                
                if (m_uiState.showOnlyOwned) {
                    // Check if owned
                    auto& inv = InventoryCore::Instance();
                    auto owned = inv.GetItemsByType(sticker.id);
                    if (owned.empty()) continue;
                }
                
                if (ImGui::Selectable(sticker.name.c_str(), sticker.id == currentSticker)) {
                    StickerPreset preset;
                    preset.stickerId = sticker.id;
                    ApplySticker(m_uiState.selectedItemId, slot, sticker.id, preset);
                }
            }
            
            ImGui::EndCombo();
        }
        
        if (currentSticker > 0) {
            ImGui::SameLine();
            if (ImGui::Button("Scrape")) {
                ScrapeSticker(m_uiState.selectedItemId, slot, 0.1f);
            }
            ImGui::SameLine();
            if (ImGui::Button("Peel")) {
                PeelSticker(m_uiState.selectedItemId, slot);
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove")) {
                RemoveSticker(m_uiState.selectedItemId, slot);
            }
        }
        
        // Position/Rotation/Scale controls for applied sticker
        if (currentSticker > 0) {
            ImGui::Indent();
            
            auto& stickerData = item.stickers[slot];
            
            if (ImGui::DragFloat2("Position", &stickerData.offset.x, 0.1f, -100.0f, 100.0f)) {
                SetStickerPosition(m_uiState.selectedItemId, slot, stickerData.offset);
            }
            
            if (ImGui::DragFloat("Rotation", &stickerData.rotation, 1.0f, 0.0f, 360.0f)) {
                SetStickerRotation(m_uiState.selectedItemId, slot, stickerData.rotation);
            }
            
            if (ImGui::DragFloat("Scale", &stickerData.scale, 0.01f, 0.1f, 5.0f)) {
                SetStickerScale(m_uiState.selectedItemId, slot, stickerData.scale);
            }
            
            if (ImGui::DragFloat("Wear", &stickerData.wear, 0.001f, 0.0f, 1.0f)) {
                SetStickerWear(m_uiState.selectedItemId, slot, stickerData.wear);
            }
            
            // Preset buttons
            if (ImGui::Button("Save Preset")) {
                StickerPreset preset;
                preset.stickerId = currentSticker;
                preset.wear = stickerData.wear;
                preset.scale = stickerData.scale;
                preset.rotation = stickerData.rotation;
                preset.offset = stickerData.offset;
                
                std::string name = fmt::format("Preset_{}_{}", GetStickerName(currentSticker), slot);
                SavePreset(name, preset);
            }
            
            ImGui::Unindent();
        }
        
        ImGui::PopID();
    }
    
    ImGui::Separator();
    
    // Presets panel
    if (ImGui::CollapsingHeader("Presets")) {
        RenderPresetsPanel();
    }
}

void StickerTool::RenderPresetsPanel() {
    if (m_presets.empty()) {
        ImGui::Text("No presets saved");
        return;
    }
    
    for (auto& [name, preset] : m_presets) {
        ImGui::PushID(name.c_str());
        
        std::string displayName = fmt::format("{} ({})", name, GetStickerName(preset.stickerId));
        if (ImGui::Selectable(displayName.c_str())) {
            m_uiState.currentPreset = preset;
        }
        
        ImGui::SameLine();
        if (ImGui::SmallButton("Delete")) {
            DeletePreset(name);
        }
        
        ImGui::PopID();
    }
}

bool StickerTool::ApplySticker(uint64_t itemId, int slot, int stickerId, const StickerPreset& preset) {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) return false;
    if (slot < 0 || slot >= 5) return false;
    
    InventoryItem item = *itemOpt;
    
    // Get default position for this weapon/slot
    auto posIt = m_stickerPositions.find(item.itemDefIndex);
    sdk::Vector2D defaultPos = {0, 0};
    float defaultScale = 1.0f;
    
    if (posIt != m_stickerPositions.end()) {
        defaultPos = posIt->second[slot].defaultPos;
        defaultScale = posIt->second[slot].defaultScale;
    }
    
    InventoryItem::StickerApplied sticker;
    sticker.stickerId = stickerId;
    sticker.slot = slot;
    sticker.wear = preset.wear > 0 ? preset.wear : 0.0f;
    sticker.scale = preset.scale > 0 ? preset.scale : defaultScale;
    sticker.rotation = preset.rotation;
    sticker.offset = preset.offset.x != 0 || preset.offset.y != 0 ? preset.offset : defaultPos;
    sticker.isPeeled = false;
    
    item.stickers[slot] = sticker;
    
    return inv.UpdateItem(itemId, item);
}

bool StickerTool::ScrapeSticker(uint64_t itemId, int slot, float amount) {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) return false;
    if (slot < 0 || slot >= 5) return false;
    
    InventoryItem item = *itemOpt;
    auto& sticker = item.stickers[slot];
    
    if (sticker.stickerId == 0) return false;
    if (sticker.isPeeled) return false;
    
    sticker.wear = std::min(1.0f, sticker.wear + amount);
    
    if (sticker.wear >= 1.0f) {
        // Start peel animation
        sticker.isPeeled = true;
        sticker.peelProgress = 0.0f;
    }
    
    return inv.UpdateItem(itemId, item);
}

bool StickerTool::PeelSticker(uint64_t itemId, int slot) {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) return false;
    if (slot < 0 || slot >= 5) return false;
    
    InventoryItem item = *itemOpt;
    auto& sticker = item.stickers[slot];
    
    if (sticker.stickerId == 0) return false;
    
    sticker.isPeeled = true;
    sticker.peelProgress = 0.0f;
    
    return inv.UpdateItem(itemId, item);
}

bool StickerTool::RemoveSticker(uint64_t itemId, int slot) {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) return false;
    if (slot < 0 || slot >= 5) return false;
    
    InventoryItem item = *itemOpt;
    item.stickers[slot] = InventoryItem::StickerApplied{};
    
    return inv.UpdateItem(itemId, item);
}

bool StickerTool::SetStickerPosition(uint64_t itemId, int slot, const sdk::Vector2D& offset) {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) return false;
    if (slot < 0 || slot >= 5) return false;
    
    InventoryItem item = *itemOpt;
    item.stickers[slot].offset = offset;
    
    return inv.UpdateItem(itemId, item);
}

bool StickerTool::SetStickerRotation(uint64_t itemId, int slot, float rotation) {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) return false;
    if (slot < 0 || slot >= 5) return false;
    
    InventoryItem item = *itemOpt;
    item.stickers[slot].rotation = fmodf(rotation, 360.0f);
    
    return inv.UpdateItem(itemId, item);
}

bool StickerTool::SetStickerScale(uint64_t itemId, int slot, float scale) {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) return false;
    if (slot < 0 || slot >= 5) return false;
    
    InventoryItem item = *itemOpt;
    item.stickers[slot].scale = std::clamp(scale, 0.1f, 5.0f);
    
    return inv.UpdateItem(itemId, item);
}

bool StickerTool::SetStickerWear(uint64_t itemId, int slot, float wear) {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) return false;
    if (slot < 0 || slot >= 5) return false;
    
    InventoryItem item = *itemOpt;
    item.stickers[slot].wear = std::clamp(wear, 0.0f, 1.0f);
    
    return inv.UpdateItem(itemId, item);
}

std::optional<StickerTool::StickerSlotData> StickerTool::GetStickerData(uint64_t itemId, int slot) {
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) return std::nullopt;
    if (slot < 0 || slot >= 5) return std::nullopt;
    
    const auto& sticker = itemOpt->stickers[slot];
    StickerSlotData data;
    data.slot = slot;
    data.stickerId = sticker.stickerId;
    data.wear = sticker.wear;
    data.scale = sticker.scale;
    data.rotation = sticker.rotation;
    data.offset = sticker.offset;
    data.isPeeled = sticker.isPeeled;
    data.peelProgress = sticker.peelProgress;
    
    return data;
}

std::vector<StickerTool::StickerSlotData> StickerTool::GetAllStickers(uint64_t itemId) {
    std::vector<StickerSlotData> result;
    
    auto& inv = InventoryCore::Instance();
    auto itemOpt = inv.GetItem(itemId);
    
    if (!itemOpt) return result;
    
    for (int i = 0; i < 5; ++i) {
        const auto& sticker = itemOpt->stickers[i];
        if (sticker.stickerId > 0) {
            StickerSlotData data;
            data.slot = i;
            data.stickerId = sticker.stickerId;
            data.wear = sticker.wear;
            data.scale = sticker.scale;
            data.rotation = sticker.rotation;
            data.offset = sticker.offset;
            data.isPeeled = sticker.isPeeled;
            data.peelProgress = sticker.peelProgress;
            result.push_back(data);
        }
    }
    
    return result;
}

std::vector<StickerTool::StickerInfo> StickerTool::GetAllStickers() const {
    return m_stickers;
}

std::optional<StickerTool::StickerInfo> StickerTool::GetStickerInfo(int stickerId) const {
    for (const auto& s : m_stickers) {
        if (s.id == stickerId) return s;
    }
    return std::nullopt;
}

std::vector<StickerTool::StickerInfo> StickerTool::GetStickersByRarity(int rarity) const {
    std::vector<StickerInfo> result;
    for (const auto& s : m_stickers) {
        if (s.rarity == rarity) result.push_back(s);
    }
    return result;
}

std::vector<StickerTool::StickerInfo> StickerTool::SearchStickers(const std::string& query) const {
    std::vector<StickerInfo> result;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (const auto& s : m_stickers) {
        std::string lowerName = s.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        if (lowerName.find(lowerQuery) != std::string::npos) {
            result.push_back(s);
        }
    }
    
    return result;
}

void StickerTool::SavePreset(const std::string& name, const StickerPreset& preset) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_presets[name] = preset;
    SavePresets();
}

std::optional<StickerTool::StickerPreset> StickerTool::LoadPreset(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_presets.find(name);
    if (it != m_presets.end()) return it->second;
    return std::nullopt;
}

std::vector<std::string> StickerTool::GetPresetNames() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> names;
    for (const auto& [name, _] : m_presets) names.push_back(name);
    return names;
}

void StickerTool::DeletePreset(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_presets.erase(name);
    SavePresets();
}

void StickerTool::UpdateScrapeAnimation(uint64_t itemId, int slot, float dt) {
    // Animation handled in render
}

void StickerTool::UpdatePeelAnimation(uint64_t itemId, int slot, float dt) {
    // Animation handled in render
}

float StickerTool::CalculateScrapeWear(float baseWear, float scrapeProgress) {
    return baseWear + scrapeProgress * (1.0f - baseWear);
}

void StickerTool::InitializeStickers() {
    // Initialize with CS2 sticker data
    // This is a subset - real implementation would have all stickers
    
    m_stickers = {
        {1, "Team Dignitas (Holo) | Katowice 2014", "icons/stickers/dignitas_holo_katowice_2014.png", 5, 50000, true, false, false, true, 1, 1, 1},
        {2, "Reason Gaming (Holo) | Katowice 2014", "icons/stickers/reason_holo_katowice_2014.png", 5, 40000, true, false, false, true, 1, 2, 1},
        {3, "iBUYPOWER (Holo) | Katowice 2014", "icons/stickers/ibp_holo_katowice_2014.png", 5, 100000, true, false, false, true, 1, 3, 1},
        {4, "Titan (Holo) | Katowice 2014", "icons/stickers/titan_holo_katowice_2014.png", 5, 80000, true, false, false, true, 1, 4, 1},
        {5, "ESL One Cologne 2014", "icons/stickers/esl_cologne_2014.png", 3, 5000, false, false, false, true, 2, 0, 1},
        {6, "DreamHack 2014", "icons/stickers/dreamhack_2014.png", 3, 3000, false, false, false, true, 3, 0, 1},
        {7, "ESL One Katowice 2015", "icons/stickers/esl_katowice_2015.png", 3, 2000, false, false, false, true, 4, 0, 1},
        {8, "ESL One Cologne 2015", "icons/stickers/esl_cologne_2015.png", 3, 1500, false, false, false, true, 5, 0, 1},
        {9, "DreamHack Cluj-Napoca 2015", "icons/stickers/dreamhack_cluj_2015.png", 3, 1000, false, false, false, true, 6, 0, 1},
        {10, "MLG Columbus 2016", "icons/stickers/mlg_columbus_2016.png", 3, 800, false, false, false, true, 7, 0, 1},
        {11, "ESL One Cologne 2016", "icons/stickers/esl_cologne_2016.png", 3, 600, false, false, false, true, 8, 0, 1},
        {12, "ELEAGUE Atlanta 2017", "icons/stickers/eleague_atlanta_2017.png", 3, 500, false, false, false, true, 9, 0, 1},
        {13, "PGL Krakow 2017", "icons/stickers/pgl_krakow_2017.png", 3, 400, false, false, false, true, 10, 0, 1},
        {14, "ELEAGUE Boston 2018", "icons/stickers/eleague_boston_2018.png", 3, 300, false, false, false, true, 11, 0, 1},
        {15, "FACEIT London 2018", "icons/stickers/faceit_london_2018.png", 3, 250, false, false, false, true, 12, 0, 1},
        {16, "IEM Katowice 2019", "icons/stickers/iem_katowice_2019.png", 3, 200, false, false, false, true, 13, 0, 1},
        {17, "StarLadder Berlin 2019", "icons/stickers/starseries_berlin_2019.png", 3, 150, false, false, false, true, 14, 0, 1},
        {18, "PGL Stockholm 2021", "icons/stickers/pgl_stockholm_2021.png", 3, 100, false, false, false, true, 15, 0, 1},
        {19, "PGL Antwerp 2022", "icons/stickers/pgl_antwerp_2022.png", 3, 80, false, false, false, true, 16, 0, 1},
        {20, "IEM Rio 2022", "icons/stickers/iem_rio_2022.png", 3, 70, false, false, false, true, 17, 0, 1},
        {21, "BLAST Paris 2023", "icons/stickers/blast_paris_2023.png", 3, 60, false, false, false, true, 18, 0, 1},
        {22, "Copenhagen 2024", "icons/stickers/copenhagen_2024.png", 3, 50, false, false, false, true, 19, 0, 1},
    };
}

int StickerTool::GetMaxSlotsForWeapon(int itemDefIndex) const {
    // Gloves have 1 slot, weapons have 4, agents have patches
    if (itemDefIndex >= 5027 && itemDefIndex <= 5035) return 1; // Gloves
    if (itemDefIndex >= 5000 && itemDefIndex < 5027) return 4; // Knives
    return 4; // Weapons
}

void StickerTool::LoadPresets() {
    std::filesystem::path presetsPath = "ViceCity/sticker_presets.json";
    if (!std::filesystem::exists(presetsPath)) return;
    
    try {
        std::ifstream file(presetsPath);
        json j;
        file >> j;
        
        for (auto& [name, presetJson] : j.items()) {
            StickerPreset preset;
            preset.stickerId = presetJson.value("stickerId", 0);
            preset.wear = presetJson.value("wear", 0.0f);
            preset.scale = presetJson.value("scale", 1.0f);
            preset.rotation = presetJson.value("rotation", 0.0f);
            preset.offset = {presetJson.value("offsetX", 0.0f), presetJson.value("offsetY", 0.0f)};
            m_presets[name] = preset;
        }
    } catch (...) {}
}

void StickerTool::SavePresets() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    json j;
    for (auto& [name, preset] : m_presets) {
        j[name] = {
            {"stickerId", preset.stickerId},
            {"wear", preset.wear},
            {"scale", preset.scale},
            {"rotation", preset.rotation},
            {"offsetX", preset.offset.x},
            {"offsetY", preset.offset.y}
        };
    }
    
    std::filesystem::create_directories("ViceCity");
    std::ofstream file("ViceCity/sticker_presets.json");
    if (file.is_open()) {
        file << j.dump(4);
    }
}

void StickerTool::InitializeStickerPositions() {
    // Default positions for each weapon type
    // This would be populated with actual weapon-specific positions
    
    // Example for AK-47 (itemDefIndex 7)
    m_stickerPositions[7] = {{
        {{10, 10}, {-50, -50}, {50, 50}, 1.0f, 0.25f, 4.0f}, // Slot 0
        {{10, 10}, {-50, -50}, {50, 50}, 1.0f, 0.25f, 4.0f}, // Slot 1
        {{10, 10}, {-50, -50}, {50, 50}, 1.0f, 0.25f, 4.0f}, // Slot 2
        {{10, 10}, {-50, -50}, {50, 50}, 1.0f, 0.25f, 4.0f}, // Slot 3
        {{0, 0}, {-10, -10}, {10, 10}, 1.0f, 0.25f, 4.0f}  // Slot 4 (gloves N/A)
    }};
    
    // Add more weapons as needed
}

std::string StickerTool::GetWeaponName(int itemDefIndex) const {
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

std::string StickerTool::GetStickerName(int stickerId) const {
    for (const auto& s : m_stickers) {
        if (s.id == stickerId) return s.name;
    }
    return fmt::format("Sticker {}", stickerId);
}

static constexpr const char* RARITY_NAMES[8] = {
    "Consumer Grade", "Industrial Grade", "Mil-Spec Grade", "Restricted",
    "Classified", "Covert", "Contraband", "Extraordinary"
};

} // namespace skinchanger