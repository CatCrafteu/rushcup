#include "core/pch.hpp"
#include "skinchanger_system/glove_manager.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "core/logger/logger.hpp"
#include "core/config/config_manager.hpp"

namespace skinchanger {

void GloveManager::Initialize() {
    InitializeGloves();
    LoadState();
    LOG_INFO(Skinchanger, "GloveManager initialized");
}

void GloveManager::Shutdown() {
    SaveState();
    LOG_INFO(Skinchanger, "GloveManager shutdown");
}

void GloveManager::Render() {
    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Gloves")) {
        // Current equipped glove
        ImGui::Text("Equipped Glove:");
        auto glove = GetGlove(m_state.equippedGlove);
        if (glove) {
            ImGui::SameLine();
            ImGui::Text("%s", glove->name.c_str());
        } else {
            ImGui::SameLine();
            ImGui::Text("None");
        }
        
        if (ImGui::BeginCombo("##glove", glove ? glove->name.c_str() : "None")) {
            if (ImGui::Selectable("None", m_state.equippedGlove == 0)) {
                EquipGlove(0);
            }
            ImGui::Separator();
            
            auto gloves = GetOwnedGloves();
            for (const auto& g : gloves) {
                bool owned = IsGloveOwned(g.id);
                std::string label = fmt::format("{} {}", g.name, owned ? "" : " (Locked)");
                if (ImGui::Selectable(label.c_str(), g.id == m_state.equippedGlove)) {
                    if (owned || g.id == 0) {
                        EquipGlove(g.id);
                    }
                }
            }
            ImGui::EndCombo();
        }
        
        // Glove skin
        if (m_state.equippedGlove > 0) {
            ImGui::Separator();
            ImGui::Text("Glove Skin:");
            
            auto paintKits = GetPaintKitsForGlove(m_state.equippedGlove);
            if (!paintKits.empty()) {
                std::string skinName = GetSkinName(m_state.paintKit);
                if (ImGui::BeginCombo("##glove_skin", skinName.c_str())) {
                    for (int pk : paintKits) {
                        if (ImGui::Selectable(GetSkinName(pk).c_str(), pk == m_state.paintKit)) {
                            SetGloveSkin(pk, m_state.wear, m_state.seed);
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            
            // Wear
            ImGui::DragFloat("Wear", &m_state.wear, 0.0001f, 0.0f, 1.0f, "%.4f");
            
            // Seed
            ImGui::DragInt("Pattern Seed", &m_state.seed, 1, 0, 1000);
            
            if (ImGui::Button("Random Seed")) {
                m_state.seed = rand() % 1001;
            }
            ImGui::SameLine();
            if (ImGui::Button("Random Wear")) {
                m_state.wear = static_cast<float>(rand() % 10000) / 10000.0f;
            }
        }
        
        // Preview
        if (m_previewGlove > 0) {
            ImGui::Separator();
            ImGui::Text("Preview: %s", GetGlove(m_previewGlove)->name.c_str());
            ImGui::Text("Skin: %s", GetSkinName(m_previewPaintKit).c_str());
            ImGui::Text("Wear: %.4f", m_previewWear);
            ImGui::Text("Seed: %d", m_previewSeed);
            
            if (ImGui::Button("Stop Preview")) {
                StopPreview();
            }
        }
    }
    ImGui::End();
}

bool GloveManager::EquipGlove(int gloveId) {
    if (gloveId > 0 && !IsGloveOwned(gloveId)) {
        LOG_WARN(Skinchanger, "Glove not owned: %d", gloveId);
        return false;
    }
    
    m_state.equippedGlove = gloveId;
    if (gloveId > 0) {
        // Reset skin to first available
        auto paintKits = GetPaintKitsForGlove(gloveId);
        if (!paintKits.empty()) {
            m_state.paintKit = paintKits[0];
            m_state.wear = 0.001f;
            m_state.seed = 0;
        }
    } else {
        m_state.paintKit = 0;
        m_state.wear = 0.0f;
        m_state.seed = 0;
    }
    
    SaveState();
    LOG_INFO(Skinchanger, "Equipped glove: %d", gloveId);
    return true;
}

bool GloveManager::UnequipGlove() {
    m_state.equippedGlove = 0;
    m_state.paintKit = 0;
    m_state.wear = 0.0f;
    m_state.seed = 0;
    SaveState();
    return true;
}

bool GloveManager::SetGloveSkin(int paintKit, float wear, int seed) {
    // Verify this paint kit is valid for current glove
    auto paintKits = GetPaintKitsForGlove(m_state.equippedGlove);
    bool valid = false;
    for (int pk : paintKits) {
        if (pk == paintKit) { valid = true; break; }
    }
    
    if (!valid && m_state.equippedGlove > 0) {
        LOG_WARN(Skinchanger, "Invalid paint kit for current glove");
        return false;
    }
    
    m_state.paintKit = paintKit;
    m_state.wear = std::clamp(wear, 0.0f, 1.0f);
    m_state.seed = std::clamp(seed, 0, 1000);
    
    SaveState();
    return true;
}

std::vector<GloveManager::Glove> GloveManager::GetAllGloves() const {
    return m_gloves;
}

std::optional<GloveManager::Glove> GloveManager::GetGlove(int id) const {
    for (const auto& glove : m_gloves) {
        if (glove.id == id) return glove;
    }
    return std::nullopt;
}

std::vector<GloveManager::Glove> GloveManager::GetOwnedGloves() const {
    std::vector<Glove> result;
    for (const auto& glove : m_gloves) {
        if (IsGloveOwned(glove.id)) result.push_back(glove);
    }
    return result;
}

std::vector<int> GloveManager::GetPaintKitsForGlove(int gloveId) const {
    for (const auto& glove : m_gloves) {
        if (glove.id == gloveId) return glove.paintKits;
    }
    return {};
}

void GloveManager::PreviewGlove(int gloveId, int paintKit, float wear, int seed) {
    m_previewGlove = gloveId;
    m_previewPaintKit = paintKit;
    m_previewWear = wear;
    m_previewSeed = seed;
}

void GloveManager::StopPreview() {
    m_previewGlove = 0;
    m_previewPaintKit = 0;
    m_previewWear = 0.0f;
    m_previewSeed = 0;
}

void GloveManager::RenderGloveModel(int gloveId, int paintKit, float wear, int seed) {
    // Would render 3D glove model
    // Placeholder for now
}

bool GloveManager::IsGloveOwned(int gloveId) const {
    if (gloveId == 0) return true;
    
    auto& inv = InventoryCore::Instance();
    auto gloves = inv.GetGloves();
    for (const auto& glove : gloves) {
        if (glove.gloveIndex == gloveId) return true;
    }
    return false;
}

std::string GloveManager::GetSkinName(int paintKit) const {
    // Would look up paint kit name
    return fmt::format("Skin #{}", paintKit);
}

void GloveManager::InitializeGloves() {
    m_gloves = {
        {5027, "Bloodhound Gloves", "models/gloves/bloodhound.mdl", "icons/gloves/bloodhound.png", 6, 100000, {100, 101, 102}, false},
        {5028, "T-Side Gloves", "models/gloves/t_side.mdl", "icons/gloves/t_side.png", 3, 50000, {103, 104}, false},
        {5029, "CT-Side Gloves", "models/gloves/ct_side.mdl", "icons/gloves/ct_side.png", 3, 50000, {105, 106}, false},
        {5030, "Sporty Gloves", "models/gloves/sporty.mdl", "icons/gloves/sporty.png", 4, 80000, {107, 108, 109}, false},
        {5031, "Slick Gloves", "models/gloves/slick.mdl", "icons/gloves/slick.png", 4, 90000, {110, 111}, false},
        {5032, "Leather Wrap Gloves", "models/gloves/leather_wrap.mdl", "icons/gloves/leather_wrap.png", 5, 120000, {112, 113}, false},
        {5033, "Motorcycle Gloves", "models/gloves/motorcycle.mdl", "icons/gloves/motorcycle.png", 5, 150000, {114, 115}, false},
        {5034, "Specialist Gloves", "models/gloves/specialist.mdl", "icons/gloves/specialist.png", 6, 200000, {116, 117}, false},
        {5035, "Hydra Gloves", "models/gloves/hydra.mdl", "icons/gloves/hydra.png", 6, 250000, {118, 119}, false},
    };
}

void GloveManager::LoadState() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto opt = config.Get<json>("skinchanger_gloves", "gloves");
    if (opt) {
        try {
            m_state.equippedGlove = opt->value("equippedGlove", 0);
            m_state.paintKit = opt->value("paintKit", 0);
            m_state.wear = opt->value("wear", 0.001f);
            m_state.seed = opt->value("seed", 0);
        } catch (...) {}
    }
}

void GloveManager::SaveState() {
    auto& config = core::config::ConfigManager::Instance();
    
    json j;
    j["equippedGlove"] = m_state.equippedGlove;
    j["paintKit"] = m_state.paintKit;
    j["wear"] = m_state.wear;
    j["seed"] = m_state.seed;
    
    config.Set("skinchanger_gloves", "gloves", j);
}

} // namespace skinchanger