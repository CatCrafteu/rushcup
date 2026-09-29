#include "core/pch.hpp"
#include "skinchanger_system/music_kit_manager.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "core/logger/logger.hpp"
#include "core/config/config_manager.hpp"

namespace skinchanger {

void MusicKitManager::Initialize() {
    InitializeMusicKits();
    LoadState();
    LOG_INFO(Skinchanger, "MusicKitManager initialized");
}

void MusicKitManager::Shutdown() {
    StopAllMusic();
    SaveState();
    LOG_INFO(Skinchanger, "MusicKitManager shutdown");
}

void MusicKitManager::Update(float dt) {
    if (m_previewPlaying && m_previewKit > 0) {
        m_previewTime += dt;
        // Update preview playback
    }
}

void MusicKitManager::Render() {
    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Music Kits")) {
        // Current equipped
        ImGui::Text("Equipped Music Kit: %s", GetMusicKitName(m_state.equippedKit).c_str());
        ImGui::Text("Equipped MVP Anthem: %s", GetMusicKitName(m_state.equippedMVP).c_str());
        ImGui::Separator();
        
        // Volume controls
        ImGui::SliderFloat("Main Menu Volume", &m_state.mainMenuVolume, 0.0f, 1.0f);
        ImGui::SliderFloat("Round Volume", &m_state.roundVolume, 0.0f, 1.0f);
        ImGui::Checkbox("Enable Main Menu", &m_state.enableMainMenu);
        ImGui::Checkbox("Enable Round Music", &m_state.enableRound);
        ImGui::Checkbox("Enable MVP Anthems", &m_state.enableMVP);
        ImGui::Separator();
        
        // Music kit list
        ImGui::Text("Available Music Kits:");
        ImGui::Separator();
        
        auto kits = GetAllMusicKits();
        for (const auto& kit : kits) {
            ImGui::PushID(kit.id);
            
            bool isEquipped = (kit.id == m_state.equippedKit);
            bool isMVP = (kit.id == m_state.equippedMVP);
            bool isOwned = IsMusicKitOwned(kit.id);
            
            std::string label = fmt::format("{} {}", kit.name, isOwned ? "" : " (Locked)");
            
            if (ImGui::Selectable(label.c_str(), isEquipped)) {
                if (isOwned) {
                    EquipMusicKit(kit.id);
                }
            }
            
            if (isEquipped) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "[EQUIPPED]");
            }
            
            if (isMVP) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[MVP]");
            }
            
            // Preview button
            ImGui::SameLine();
            if (ImGui::SmallButton("Preview")) {
                if (IsPreviewing() && GetPreviewKit() == kit.id) {
                    StopPreview();
                } else {
                    PreviewMusicKit(kit.id);
                }
            }
            
            if (IsPreviewing() && GetPreviewKit() == kit.id) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "[PLAYING]");
            }
            
            ImGui::PopID();
        }
        
        ImGui::Separator();
        
        // Stop preview button
        if (IsPreviewing()) {
            if (ImGui::Button("Stop Preview")) {
                StopPreview();
            }
        }
    }
    ImGui::End();
}

bool MusicKitManager::EquipMusicKit(int kitId) {
    auto kit = GetMusicKit(kitId);
    if (!kit) return false;
    
    if (!IsMusicKitOwned(kitId)) {
        LOG_WARN(Skinchanger, "Music kit not owned: %d", kitId);
        return false;
    }
    
    m_state.equippedKit = kitId;
    SaveState();
    PlayMainMenu();
    LOG_INFO(Skinchanger, "Equipped music kit: %d", kitId);
    return true;
}

bool MusicKitManager::EquipMVP(int kitId) {
    auto kit = GetMusicKit(kitId);
    if (!kit) return false;
    
    if (!IsMusicKitOwned(kitId)) {
        LOG_WARN(Skinchanger, "Music kit not owned: %d", kitId);
        return false;
    }
    
    m_state.equippedMVP = kitId;
    SaveState();
    LOG_INFO(Skinchanger, "Equipped MVP anthem: %d", kitId);
    return true;
}

bool MusicKitManager::UnequipMusicKit() {
    m_state.equippedKit = 0;
    SaveState();
    StopAllMusic();
    return true;
}

bool MusicKitManager::UnequipMVP() {
    m_state.equippedMVP = 0;
    SaveState();
    return true;
}

void MusicKitManager::PreviewMusicKit(int kitId) {
    auto kit = GetMusicKit(kitId);
    if (!kit) return;
    
    StopPreview();
    m_previewKit = kitId;
    m_previewTime = 0.0f;
    m_previewPlaying = true;
    
    // Play main menu preview
    if (!kit->mainMenuPath.empty()) {
        PlayMusic(kit->mainMenuPath, m_state.mainMenuVolume);
    }
}

void MusicKitManager::StopPreview() {
    m_previewKit = 0;
    m_previewTime = 0.0f;
    m_previewPlaying = false;
    StopAllMusic();
}

std::vector<MusicKitManager::MusicKit> MusicKitManager::GetAllMusicKits() const {
    return m_musicKits;
}

std::optional<MusicKitManager::MusicKit> MusicKitManager::GetMusicKit(int id) const {
    for (const auto& kit : m_musicKits) {
        if (kit.id == id) return kit;
    }
    return std::nullopt;
}

std::vector<MusicKitManager::MusicKit> MusicKitManager::GetOwnedMusicKits() const {
    std::vector<MusicKit> result;
    for (const auto& kit : m_musicKits) {
        if (IsMusicKitOwned(kit.id)) {
            result.push_back(kit);
        }
    }
    return result;
}

std::vector<MusicKitManager::MusicKit> MusicKitManager::GetMusicKitsByRarity(int rarity) const {
    std::vector<MusicKit> result;
    for (const auto& kit : m_musicKits) {
        if (kit.rarity == rarity) result.push_back(kit);
    }
    return result;
}

void MusicKitManager::PlayMainMenu() {
    if (!m_state.enableMainMenu) return;
    
    if (m_state.equippedKit > 0) {
        auto kit = GetMusicKit(m_state.equippedKit);
        if (kit && !kit->mainMenuPath.empty()) {
            PlayMusic(kit->mainMenuPath, m_state.mainMenuVolume);
        }
    }
}

void MusicKitManager::PlayRoundStart() {
    if (!m_state.enableRound) return;
    
    if (m_state.equippedKit > 0) {
        auto kit = GetMusicKit(m_state.equippedKit);
        if (kit && !kit->roundStartPath.empty()) {
            PlayMusic(kit->roundStartPath, m_state.roundVolume);
        }
    }
}

void MusicKitManager::PlayRoundEnd(bool won) {
    if (!m_state.enableRound) return;
    
    if (m_state.equippedKit > 0) {
        auto kit = GetMusicKit(m_state.equippedKit);
        if (kit) {
            const std::string& path = won ? kit->roundEndPath : kit->roundEndPath;
            if (!path.empty()) {
                PlayMusic(path, m_state.roundVolume);
            }
        }
    }
}

void MusicKitManager::PlayBombPlant() {
    if (!m_state.enableRound) return;
    
    if (m_state.equippedKit > 0) {
        auto kit = GetMusicKit(m_state.equippedKit);
        if (kit && !kit->bombPlantPath.empty()) {
            PlayMusic(kit->bombPlantPath, m_state.roundVolume);
        }
    }
}

void MusicKitManager::PlayBombDefuse() {
    if (!m_state.enableRound) return;
    
    if (m_state.equippedKit > 0) {
        auto kit = GetMusicKit(m_state.equippedKit);
        if (kit && !kit->bombDefusePath.empty()) {
            PlayMusic(kit->bombDefusePath, m_state.roundVolume);
        }
    }
}

void MusicKitManager::PlayBombExplode() {
    if (!m_state.enableRound) return;
    
    if (m_state.equippedKit > 0) {
        auto kit = GetMusicKit(m_state.equippedKit);
        if (kit && !kit->bombExplodePath.empty()) {
            PlayMusic(kit->bombExplodePath, m_state.roundVolume);
        }
    }
}

void MusicKitManager::PlayTenSecondWarning() {
    if (!m_state.enableRound) return;
    
    if (m_state.equippedKit > 0) {
        auto kit = GetMusicKit(m_state.equippedKit);
        if (kit && !kit->roundTenSecPath.empty()) {
            PlayMusic(kit->roundTenSecPath, m_state.roundVolume);
        }
    }
}

void MusicKitManager::PlayMVP() {
    if (!m_state.enableMVP) return;
    
    if (m_state.equippedMVP > 0) {
        auto kit = GetMusicKit(m_state.equippedMVP);
        if (kit && !kit->mvpPath.empty()) {
            PlayMusic(kit->mvpPath, m_state.roundVolume);
        }
    }
}

void MusicKitManager::InitializeMusicKits() {
    m_musicKits = {
        {0, "Default", "Default CS2 music", "", "", "", "", "", "", "", "", 0, 0, true},
        {1, "Sharp Dressed Man", "ZZ Top", "music/sharp_dressed_man_main.mp3", "music/sharp_dressed_man_round_start.mp3", "music/sharp_dressed_man_round_end.mp3", "music/sharp_dressed_man_bomb_plant.mp3", "music/sharp_dressed_man_bomb_defuse.mp3", "music/sharp_dressed_man_bomb_explode.mp3", "music/sharp_dressed_man_10sec.mp3", "music/sharp_dressed_man_mvp.mp3", 3, 5000, false},
        {2, "Dust 2", "Original CS music", "music/dust2_main.mp3", "music/dust2_round_start.mp3", "music/dust2_round_end.mp3", "music/dust2_bomb_plant.mp3", "music/dust2_bomb_defuse.mp3", "music/dust2_bomb_explode.mp3", "music/dust2_10sec.mp3", "music/dust2_mvp.mp3", 2, 3000, false},
        {3, "The Fragrance of Dark Coffee", "Original CS music", "music/dark_coffee_main.mp3", "music/dark_coffee_round_start.mp3", "music/dark_coffee_round_end.mp3", "music/dark_coffee_bomb_plant.mp3", "music/dark_coffee_bomb_defuse.mp3", "music/dark_coffee_bomb_explode.mp3", "music/dark_coffee_10sec.mp3", "music/dark_coffee_mvp.mp3", 2, 3000, false},
        {4, "Awaken", "Original CS music", "music/awaken_main.mp3", "music/awaken_round_start.mp3", "music/awaken_round_end.mp3", "music/awaken_bomb_plant.mp3", "music/awaken_bomb_defuse.mp3", "music/awaken_bomb_explode.mp3", "music/awaken_10sec.mp3", "music/awaken_mvp.mp3", 3, 5000, false},
        {5, "High Noon", "Original CS music", "music/high_noon_main.mp3", "music/high_noon_round_start.mp3", "music/high_noon_round_end.mp3", "music/high_noon_bomb_plant.mp3", "music/high_noon_bomb_defuse.mp3", "music/high_noon_bomb_explode.mp3", "music/high_noon_10sec.mp3", "music/high_noon_mvp.mp3", 3, 5000, false},
        {6, "Agency", "Original CS music", "music/agency_main.mp3", "music/agency_round_start.mp3", "music/agency_round_end.mp3", "music/agency_bomb_plant.mp3", "music/agency_bomb_defuse.mp3", "music/agency_bomb_explode.mp3", "music/agency_10sec.mp3", "music/agency_mvp.mp3", 4, 10000, false},
        {7, "Czar", "Original CS music", "music/czar_main.mp3", "music/czar_round_start.mp3", "music/czar_round_end.mp3", "music/czar_bomb_plant.mp3", "music/czar_bomb_defuse.mp3", "music/czar_bomb_explode.mp3", "music/czar_10sec.mp3", "music/czar_mvp.mp3", 5, 25000, false},
        {8, "Disco", "Original CS music", "music/disco_main.mp3", "music/disco_round_start.mp3", "music/disco_round_end.mp3", "music/disco_bomb_plant.mp3", "music/disco_bomb_defuse.mp3", "music/disco_bomb_explode.mp3", "music/disco_10sec.mp3", "music/disco_mvp.mp3", 3, 5000, false},
        {9, "Metal", "Original CS music", "music/metal_main.mp3", "music/metal_round_start.mp3", "music/metal_round_end.mp3", "music/metal_bomb_plant.mp3", "music/metal_bomb_defuse.mp3", "music/metal_bomb_explode.mp3", "music/metal_10sec.mp3", "music/metal_mvp.mp3", 3, 5000, false},
        {10, "Gothic", "Original CS music", "music/gothic_main.mp3", "music/gothic_round_start.mp3", "music/gothic_round_end.mp3", "music/gothic_bomb_plant.mp3", "music/gothic_bomb_defuse.mp3", "music/gothic_bomb_explode.mp3", "music/gothic_10sec.mp3", "music/gothic_mvp.mp3", 3, 5000, false},
        {11, "Jazz", "Original CS music", "music/jazz_main.mp3", "music/jazz_round_start.mp3", "music/jazz_round_end.mp3", "music/jazz_bomb_plant.mp3", "music/jazz_bomb_defuse.mp3", "music/jazz_bomb_explode.mp3", "music/jazz_10sec.mp3", "music/jazz_mvp.mp3", 3, 5000, false},
        {12, "Electronic", "Original CS music", "music/electronic_main.mp3", "music/electronic_round_start.mp3", "music/electronic_round_end.mp3", "music/electronic_bomb_plant.mp3", "music/electronic_bomb_defuse.mp3", "music/electronic_bomb_explode.mp3", "music/electronic_10sec.mp3", "music/electronic_mvp.mp3", 4, 10000, false},
    };
}

bool MusicKitManager::IsMusicKitOwned(int kitId) const {
    // Check inventory
    auto& inv = InventoryCore::Instance();
    auto items = inv.GetMusicKits();
    for (const auto& item : items) {
        if (item.musicIndex == kitId) return true;
    }
    
    // Default kit always owned
    if (kitId == 0) return true;
    
    return false;
}

void MusicKitManager::LoadState() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto opt = config.Get<json>("skinchanger_music", "music_kits");
    if (opt) {
        try {
            m_state.equippedKit = opt->value("equippedKit", 0);
            m_state.equippedMVP = opt->value("equippedMVP", 0);
            m_state.mainMenuVolume = opt->value("mainMenuVolume", 1.0f);
            m_state.roundVolume = opt->value("roundVolume", 1.0f);
            m_state.enableMainMenu = opt->value("enableMainMenu", true);
            m_state.enableRound = opt->value("enableRound", true);
            m_state.enableMVP = opt->value("enableMVP", true);
        } catch (...) {}
    }
}

void MusicKitManager::SaveState() {
    auto& config = core::config::ConfigManager::Instance();
    
    json j;
    j["equippedKit"] = m_state.equippedKit;
    j["equippedMVP"] = m_state.equippedMVP;
    j["mainMenuVolume"] = m_state.mainMenuVolume;
    j["roundVolume"] = m_state.roundVolume;
    j["enableMainMenu"] = m_state.enableMainMenu;
    j["enableRound"] = m_state.enableRound;
    j["enableMVP"] = m_state.enableMVP;
    
    config.Set("skinchanger_music", "music_kits", j);
}

void MusicKitManager::PlayMusic(const std::string& path, float volume) {
    // Would play music file
    // This is a placeholder - real implementation would use audio engine
    LOG_DEBUG(Skinchanger, "Playing music: %s (vol: %.2f)", path.c_str(), volume);
}

void MusicKitManager::StopAllMusic() {
    // Would stop all music
    LOG_DEBUG(Skinchanger, "Stopping all music");
}

std::string MusicKitManager::GetMusicKitName(int kitId) const {
    auto kit = GetMusicKit(kitId);
    if (kit) return kit->name;
    return "Unknown";
}

} // namespace skinchanger