#include "core/pch.hpp"
#include "skinchanger_system/case_opening.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "core/logger/logger.hpp"
#include "core/config/config_manager.hpp"

namespace skinchanger {

void CaseOpening::Initialize() {
    InitializeCases();
    InitializeKeys();
    InitializeCaseDrops();
    InitializeRNG();
    LOG_INFO(Skinchanger, "CaseOpening initialized");
}

void CaseOpening::Shutdown() {
    LOG_INFO(Skinchanger, "CaseOpening shutdown");
}

void CaseOpening::Update(float dt) {
    if (!m_session.isRolling) return;
    
    m_session.rollStartTime += dt;
    float progress = m_session.rollStartTime / m_session.rollDuration;
    
    if (progress >= 1.0f) {
        progress = 1.0f;
        m_session.isRolling = false;
        m_session.rollComplete = true;
        
        // Give item to player
        GiveWinningItem();
        
        // Play reveal sound
        PlayRevealSound();
        
        // Check for rare item
        if (m_session.rareItemIndex >= 0) {
            PlayRareSound(m_session.rareItemIndex);
        }
    }
    
    // Update roll animation
    UpdateRollAnimation(dt);
}

void CaseOpening::Render() {
    if (!m_session.isRolling && !m_session.rollComplete) return;
    
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(800, 400));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
    
    if (ImGui::Begin("Case Opening", nullptr, flags)) {
        // Case info
        auto caseInfo = GetCaseInfo(m_session.caseId);
        if (caseInfo) {
            ImGui::Text("Opening: %s", caseInfo->name.c_str());
        }
        
        ImGui::Separator();
        
        // Roll animation
        RenderRollAnimation();
        
        if (m_session.rollComplete) {
            ImGui::Separator();
            ImGui::Spacing();
            
            // Show won item
            RenderWonItem();
            
            ImGui::Spacing();
            if (ImGui::Button("Continue", ImVec2(120, 40))) {
                m_session.rollComplete = false;
                m_session = CaseOpeningSession{};
            }
            ImGui::SameLine();
            if (ImGui::Button("Open Another", ImVec2(120, 40))) {
                // Reopen same case
                m_session = CaseOpeningSession{};
                StartOpening(m_session.caseId, m_session.keyId);
            }
        }
    }
    ImGui::End();
}

bool CaseOpening::StartOpening(int caseId, int keyId) {
    auto caseInfo = GetCaseInfo(caseId);
    auto keyInfo = GetKeyInfo(keyId);
    
    if (!caseInfo || !keyInfo) {
        LOG_WARN(Skinchanger, "Invalid case or key");
        return false;
    }
    
    auto& inv = InventoryCore::Instance();
    if (!inv.SpendBalance(caseInfo->price + keyInfo->price)) {
        LOG_WARN(Skinchanger, "Insufficient balance");
        return false;
    }
    
    m_session = CaseOpeningSession{};
    m_session.caseId = caseId;
    m_session.keyId = keyId;
    m_session.rollDuration = 3.5f + (std::rand() % 1500) / 1000.0f; // 3.5-5.0s
    m_session.rollStartTime = 0.0f;
    m_session.isRolling = true;
    m_session.rollComplete = false;
    m_session.currentOffset = 0.0f;
    m_session.itemWidth = 120.0f;
    m_session.visibleItems = 9;
    m_session.slowdownStart = 0.7f;
    m_session.slowdownEnd = 0.95f;
    
    // Generate roll sequence
    m_session.rollSequence = GenerateRollSequence(caseId, 50);
    m_session.winnerIndex = SelectWinner(m_session.rollSequence, caseId);
    
    // Find rare item in sequence
    auto drops = GetCaseDrops(caseId);
    for (size_t i = 0; i < m_session.rollSequence.size(); ++i) {
        for (const auto& drop : drops) {
            if (drop.itemId == m_session.rollSequence[i] && drop.rarity >= 5) { // Covert or higher
                m_session.rareItemIndex = static_cast<int>(i);
                break;
            }
        }
        if (m_session.rareItemIndex >= 0) break;
    }
    
    PlayRollSound();
    LOG_INFO(Skinchanger, "Started case opening: case=%d, key=%d", caseId, keyId);
    return true;
}

void CaseOpening::StopOpening() {
    m_session.isRolling = false;
    m_session.rollComplete = false;
}

std::vector<int> CaseOpening::GenerateRollSequence(int caseId, int count) {
    std::vector<int> sequence;
    sequence.reserve(count);
    
    auto drops = GetCaseDrops(caseId);
    if (drops.empty()) return sequence;
    
    // Weighted random selection based on rarity
    for (int i = 0; i < count; ++i) {
        float roll = m_rng() / static_cast<float>(m_rng.max());
        float cumulative = 0.0f;
        
        for (const auto& drop : drops) {
            cumulative += GetRarityChance(drop.rarity);
            if (roll <= cumulative) {
                sequence.push_back(drop.itemId);
                break;
            }
        }
        
        // Fallback
        if (sequence.size() != i + 1) {
            sequence.push_back(drops[0].itemId);
        }
    }
    
    // Ensure winner is at the end (or near end for animation)
    if (!sequence.empty()) {
        // We'll handle winner positioning in animation
    }
    
    return sequence;
}

int CaseOpening::SelectWinner(const std::vector<int>& rollSequence, int caseId) {
    // Winner is the last item in sequence
    if (rollSequence.empty()) return 0;
    return static_cast<int>(rollSequence.size()) - 1;
}

float CaseOpening::GetRarityChance(int rarity) const {
    if (rarity >= 0 && rarity < 8) {
        return RARITY_CHANCES[rarity];
    }
    return 0.0f;
}

void CaseOpening::UpdateRollAnimation(float dt) {
    if (!m_session.isRolling) return;
    
    float progress = m_session.rollStartTime / m_session.rollDuration;
    
    // Calculate speed based on progress
    float speed = 1.0f;
    if (progress > m_session.slowdownStart) {
        float slowdownProgress = (progress - m_session.slowdownStart) / (m_session.slowdownEnd - m_session.slowdownStart);
        slowdownProgress = std::clamp(slowdownProgress, 0.0f, 1.0f);
        speed = 1.0f - slowdownProgress * 0.95f; // Slow down to 5% speed
    }
    
    // Update offset
    m_session.currentOffset += speed * dt * 1000.0f; // pixels per second
    
    // Check for rare item glow
    if (m_session.rareItemIndex >= 0) {
        // Trigger glow when rare item is centered
        float itemCenterOffset = m_session.rareItemIndex * m_session.itemWidth + m_session.itemWidth * 0.5f;
        float screenCenter = m_session.visibleItems * m_session.itemWidth * 0.5f;
        
        if (std::abs(m_session.currentOffset - itemCenterOffset + screenCenter) < 50.0f) {
            m_session.showRareGlow = true;
            m_session.rareGlowIntensity = std::min(1.0f, m_session.rareGlowIntensity + dt * 5.0f);
        } else {
            m_session.showRareGlow = false;
            m_session.rareGlowIntensity = std::max(0.0f, m_session.rareGlowIntensity - dt * 2.0f);
        }
    }
}

void CaseOpening::TriggerRareGlow(int index) {
    m_session.rareItemIndex = index;
    m_session.showRareGlow = true;
    m_session.rareGlowIntensity = 1.0f;
}

void CaseOpening::PlayRollSound() {
    // Would play rolling sound
    LOG_DEBUG(Skinchanger, "Playing roll sound");
}

void CaseOpening::PlayRevealSound() {
    // Would play reveal sound
    LOG_DEBUG(Skinchanger, "Playing reveal sound");
}

void CaseOpening::PlayRareSound(int rarity) {
    // Would play rare item sound
    LOG_DEBUG(Skinchanger, "Playing rare sound for rarity: {}", rarity);
}

void CaseOpening::GiveWinningItem() {
    if (m_session.winnerIndex < 0 || m_session.winnerIndex >= (int)m_session.rollSequence.size()) return;
    
    int wonItemId = m_session.rollSequence[m_session.winnerIndex];
    auto& inv = InventoryCore::Instance();
    
    // Create item with random wear/seed
    InventoryItem item;
    item.itemDefIndex = wonItemId;
    item.paintKit = 0; // Would be determined by case drop
    item.wear = static_cast<float>(m_rng() % 10000) / 10000.0f; // 0.0000 - 0.9999
    item.seed = m_rng() % 1001; // 0-1000
    item.statTrak = (m_rng() % 10 == 0) ? 0 : -1; // 10% chance for StatTrak
    item.rarity = 0; // Would be determined by drop
    
    uint64_t newItemId = inv.AddItem(item);
    LOG_INFO(Skinchanger, "Won item: %d (ID: %llu)", wonItemId, newItemId);
}

void CaseOpening::RenderRollAnimation() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    float centerX = pos.x + ImGui::GetContentRegionAvail().x * 0.5f;
    float centerY = pos.y + 100;
    
    // Draw items in roll
    int halfVisible = m_session.visibleItems / 2;
    float startOffset = m_session.currentOffset - halfVisible * m_session.itemWidth;
    
    for (int i = 0; i < (int)m_session.rollSequence.size(); ++i) {
        float itemX = centerX - startOffset + i * m_session.itemWidth;
        
        // Only draw visible items
        if (itemX < pos.x - m_session.itemWidth || itemX > pos.x + ImGui::GetContentRegionAvail().x + m_session.itemWidth) {
            continue;
        }
        
        ImVec2 itemPos = ImVec2(itemX - m_session.itemWidth * 0.5f, centerY - 50);
        ImVec2 itemSize = ImVec2(m_session.itemWidth * 0.9f, 100);
        
        // Item background
        ImU32 bgColor = IM_COL32(36, 30, 36, 255);
        ImU32 borderColor = IM_COL32(55, 45, 55, 255);
        
        // Highlight winner
        if (m_session.rollComplete && i == m_session.winnerIndex) {
            borderColor = IM_COL32(255, 215, 0, 255); // Gold
            bgColor = IM_COL32(48, 40, 48, 255);
        }
        
        // Rare glow
        if (m_session.showRareGlow && i == m_session.rareItemIndex) {
            float glowAlpha = m_session.rareGlowIntensity * 0.5f;
            ImU32 glowColor = IM_COL32(255, 215, 0, static_cast<int>(255 * glowAlpha));
            drawList->AddRectFilled(
                ImVec2(itemPos.x - 4, itemPos.y - 4),
                ImVec2(itemPos.x + itemSize.x + 4, itemPos.y + itemSize.y + 4),
                glowColor, 8.0f
            );
        }
        
        drawList->AddRectFilled(itemPos, ImVec2(itemPos.x + itemSize.x, itemPos.y + itemSize.y), bgColor, 6.0f);
        drawList->AddRect(itemPos, ImVec2(itemPos.x + itemSize.x, itemPos.y + itemSize.y), borderColor, 6.0f, 0, 2.0f);
        
        // Item name (would be actual item name)
        std::string itemName = fmt::format("Item {}", m_session.rollSequence[i]);
        ImVec2 textSize = ImGui::CalcTextSize(itemName.c_str());
        drawList->AddText(
            ImVec2(itemPos.x + (itemSize.x - textSize.x) * 0.5f, itemPos.y + 10),
            IM_COL32(255, 245, 240, 255), itemName.c_str()
        );
        
        // Rarity color bar
        // Would use actual rarity from drop data
        ImU32 rarityColor = IM_COL32(176, 179, 184, 255); // Consumer gray
        drawList->AddRectFilled(
            ImVec2(itemPos.x, itemPos.y + itemSize.y - 4),
            ImVec2(itemPos.x + itemSize.x, itemPos.y + itemSize.y),
            rarityColor, 0.0f, ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight
        );
    }
    
    // Center line indicator
    drawList->AddLine(
        ImVec2(centerX, centerY - 60),
        ImVec2(centerX, centerY + 60),
        IM_COL32(255, 95, 155, 255), 3.0f
    );
    
    ImGui::Dummy(ImVec2(0, 200));
}

void CaseOpening::RenderWonItem() {
    if (m_session.winnerIndex < 0 || m_session.winnerIndex >= (int)m_session.rollSequence.size()) return;
    
    int wonItemId = m_session.rollSequence[m_session.winnerIndex];
    
    ImGui::Text("Congratulations! You won:");
    ImGui::Separator();
    
    // Item display
    ImGui::BeginChild("##wonitem", ImVec2(300, 200), true);
    
    // Would show actual item with image, name, wear, stickers, etc.
    ImGui::Text("Item ID: %d", wonItemId);
    ImGui::Text("Rarity: Covert"); // Would be actual rarity
    ImGui::Text("Wear: 0.0012");
    ImGui::Text("Seed: 123");
    ImGui::Text("StatTrak: No");
    
    ImGui::EndChild();
}

std::optional<CaseOpening::CaseInfo> CaseOpening::GetCaseInfo(int caseId) const {
    for (const auto& c : m_cases) {
        if (c.id == caseId) return c;
    }
    return std::nullopt;
}

std::optional<CaseOpening::KeyInfo> CaseOpening::GetKeyInfo(int keyId) const {
    for (const auto& k : m_keys) {
        if (k.id == keyId) return k;
    }
    return std::nullopt;
}

std::vector<CaseOpening::CaseInfo> CaseOpening::GetAllCases() const {
    return m_cases;
}

std::vector<CaseOpening::KeyInfo> CaseOpening::GetAllKeys() const {
    return m_keys;
}

std::vector<CaseOpening::CaseDrop> CaseOpening::GetCaseDrops(int caseId) const {
    auto it = m_caseDrops.find(caseId);
    if (it == m_caseDrops.end()) return {};
    return it->second;
}

void CaseOpening::InitializeCases() {
    m_cases = {
        {1, "Prisma Case", "icons/cases/prisma.png", {}, {}, 1, 1000},
        {2, "Danger Zone Case", "icons/cases/danger_zone.png", {}, {}, 1, 1000},
        {3, "Clutch Case", "icons/cases/clutch.png", {}, {}, 1, 1000},
        {4, "Horizon Case", "icons/cases/horizon.png", {}, {}, 1, 1000},
        {5, "CS20 Case", "icons/cases/cs20.png", {}, {}, 1, 1500},
        {6, "Shattered Web Case", "icons/cases/shattered_web.png", {}, {}, 1, 1500},
        {7, "Gamma 2 Case", "icons/cases/gamma2.png", {}, {}, 1, 1000},
        {8, "Gamma Case", "icons/cases/gamma.png", {}, {}, 1, 1000},
        {9, "Chroma 3 Case", "icons/cases/chroma3.png", {}, {}, 1, 1000},
        {10, "Chroma 2 Case", "icons/cases/chroma2.png", {}, {}, 1, 1000},
        {11, "Chroma Case", "icons/cases/chroma.png", {}, {}, 1, 1000},
        {12, "Operation Hydra Case", "icons/cases/hydra.png", {}, {}, 1, 1500},
        {13, "Spectrum 2 Case", "icons/cases/spectrum2.png", {}, {}, 1, 1000},
        {14, "Spectrum Case", "icons/cases/spectrum.png", {}, {}, 1, 1000},
        {15, "Operation Wildfire Case", "icons/cases/wildfire.png", {}, {}, 1, 1500},
        {16, "Falchion Case", "icons/cases/falchion.png", {}, {}, 1, 1000},
        {17, "Shadow Case", "icons/cases/shadow.png", {}, {}, 1, 1000},
        {18, "Revolver Case", "icons/cases/revolver.png", {}, {}, 1, 1000},
        {19, "Operation Vanguard Case", "icons/cases/vanguard.png", {}, {}, 1, 1500},
        {20, "Chop Shop Case", "icons/cases/chop_shop.png", {}, {}, 1, 1000},
    };
}

void CaseOpening::InitializeKeys() {
    m_keys = {
        {1, "Prisma Key", "icons/keys/prisma.png", 2500},
        {2, "Danger Zone Key", "icons/keys/danger_zone.png", 2500},
        {3, "Clutch Key", "icons/keys/clutch.png", 2500},
        {4, "Horizon Key", "icons/keys/horizon.png", 2500},
        {5, "CS20 Key", "icons/keys/cs20.png", 3000},
        {6, "Shattered Web Key", "icons/keys/shattered_web.png", 3000},
        {7, "Gamma 2 Key", "icons/keys/gamma2.png", 2500},
        {8, "Gamma Key", "icons/keys/gamma.png", 2500},
        {9, "Chroma 3 Key", "icons/keys/chroma3.png", 2500},
        {10, "Chroma 2 Key", "icons/keys/chroma2.png", 2500},
        {11, "Chroma Key", "icons/keys/chroma.png", 2500},
        {12, "Hydra Key", "icons/keys/hydra.png", 3000},
        {13, "Spectrum 2 Key", "icons/keys/spectrum2.png", 2500},
        {14, "Spectrum Key", "icons/keys/spectrum.png", 2500},
        {15, "Wildfire Key", "icons/keys/wildfire.png", 3000},
        {16, "Falchion Key", "icons/keys/falchion.png", 2500},
        {17, "Shadow Key", "icons/keys/shadow.png", 2500},
        {18, "Revolver Key", "icons/keys/revolver.png", 2500},
        {19, "Vanguard Key", "icons/keys/vanguard.png", 3000},
        {20, "Chop Shop Key", "icons/keys/chop_shop.png", 2500},
    };
}

void CaseOpening::InitializeCaseDrops() {
    // For each case, define possible drops
    // This is simplified - real implementation would have full drop tables
    
    for (const auto& caseInfo : m_cases) {
        std::vector<CaseDrop> drops;
        
        // Add some example drops
        // In reality, each case has specific drop tables
        
        // Common drops (Consumer/Industrial)
        drops.push_back({1, 100, 0.0f, 0, 0, false, false}); // Consumer skin
        drops.push_back({2, 200, 0.0f, 0, 1, false, false}); // Industrial skin
        
        // Rare drops (Mil-Spec/Restricted)
        drops.push_back({3, 300, 0.0f, 0, 2, false, false}); // Mil-Spec
        drops.push_back({4, 400, 0.0f, 0, 3, false, false}); // Restricted
        
        // Very rare (Classified/Covert)
        drops.push_back({5, 500, 0.0f, 0, 4, false, false}); // Classified
        drops.push_back({6, 600, 0.0f, 0, 5, false, false}); // Covert
        
        // Extremely rare (Knives/Gloves)
        drops.push_back({500, 100, 0.0f, 0, 6, false, false}); // Knife
        drops.push_back({5027, 200, 0.0f, 0, 6, false, false}); // Glove
        
        m_caseDrops[caseInfo.id] = drops;
    }
}

void CaseOpening::InitializeRNG() {
    std::random_device rd;
    m_rng.seed(rd());
}

} // namespace skinchanger