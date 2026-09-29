#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <random>
#include <chrono>
#include <mutex>
#include "../../core/sdk/structs.hpp"
#include "../../core/config/config_manager.hpp"
#include "inventory_core.hpp"

namespace skinchanger {

// Case opening simulation (memesense parity)
class CaseOpening {
public:
    struct CaseOpeningSession {
        int caseId = 0;
        int keyId = 0;
        std::vector<int> rollSequence;     // Item IDs in roll order
        int winnerIndex = 0;               // Index of winning item
        float rollDuration = 3.5f;         // 3-5 seconds
        float rollStartTime = 0.0f;
        bool isRolling = false;
        bool rollComplete = false;
        float currentOffset = 0.0f;        // For animation
        float itemWidth = 120.0f;          // Width of each item in roll
        int visibleItems = 9;              // Items visible at once
        float slowdownStart = 0.7f;        // Start slowing at 70%
        float slowdownEnd = 0.95f;         // Stop at 95%
        int rareItemIndex = -1;            // Index of rare item (gold/red)
        bool showRareGlow = false;
        float rareGlowIntensity = 0.0f;
    };
    
    struct CaseDrop {
        int itemId = 0;
        int paintKit = 0;
        float wear = 0.0f;
        int seed = 0;
        int rarity = 0;
        bool isStatTrak = false;
        bool isSouvenir = false;
    };
    
    static CaseOpening& Instance() {
        static CaseOpening instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Update(float dt);
    void Render();
    
    // Start case opening
    bool StartOpening(int caseId, int keyId);
    void StopOpening();
    
    // Roll generation
    std::vector<int> GenerateRollSequence(int caseId, int count = 50);
    int SelectWinner(const std::vector<int>& rollSequence, int caseId);
    float GetRarityChance(int rarity) const;
    
    // Animation
    void UpdateRollAnimation(float dt);
    void TriggerRareGlow(int index);
    void PlayRollSound();
    void PlayRevealSound();
    void PlayRareSound(int rarity);
    
    // State
    CaseOpeningSession& GetSession() { return m_session; }
    const CaseOpeningSession& GetSession() const { return m_session; }
    bool IsOpening() const { return m_session.isRolling; }
    bool IsComplete() const { return m_session.rollComplete; }
    
    // Case/Key definitions
    struct CaseInfo {
        int id = 0;
        std::string name;
        std::string iconPath;
        std::vector<int> possibleItems; // Item IDs
        std::vector<float> rarityWeights; // Per rarity
        int keyId = 0;
        uint64_t price = 0;
    };
    
    struct KeyInfo {
        int id = 0;
        std::string name;
        std::string iconPath;
        uint64_t price = 0;
    };
    
    std::optional<CaseInfo> GetCaseInfo(int caseId) const;
    std::optional<KeyInfo> GetKeyInfo(int keyId) const;
    std::vector<CaseInfo> GetAllCases() const;
    std::vector<KeyInfo> GetAllKeys() const;
    
    // Case drops (what you can get)
    std::vector<CaseDrop> GetCaseDrops(int caseId) const;

private:
    CaseOpening() = default;
    CaseOpeningSession m_session;
    std::mt19937 m_rng;
    std::mutex m_mutex;
    
    std::vector<CaseInfo> m_cases;
    std::vector<KeyInfo> m_keys;
    std::unordered_map<int, std::vector<CaseDrop>> m_caseDrops;
    
    void InitializeCases();
    void InitializeKeys();
    void InitializeCaseDrops();
    void InitializeRNG();
    
    // Rarity chances (approximate CS2 values)
    static constexpr float RARITY_CHANCES[8] = {
        0.7997f, // Consumer (Gray)
        0.1598f, // Industrial (Light Blue)
        0.0320f, // Mil-Spec (Blue)
        0.0064f, // Restricted (Purple)
        0.0016f, // Classified (Pink)
        0.0003f, // Covert (Red)
        0.0002f, // Contraband (Gold)
        0.0000f  // Extra
    };
    
    static constexpr const char* RARITY_NAMES[8] = {
        "Consumer", "Industrial", "Mil-Spec", "Restricted",
        "Classified", "Covert", "Contraband", "Extra"
    };
    
    static constexpr uint32_t RARITY_COLORS[8] = {
        0xB0B3B8FF, // Gray
        0x5E98D9FF, // Light Blue
        0x4B69FFFF, // Blue
        0x8847FFFF, // Purple
        0xD32CE6FF, // Pink
        0xEB4B4BFF, // Red
        0xE4AE39FF, // Gold
        0xFFFFFFFF  // White
    };
};

} // namespace skinchanger