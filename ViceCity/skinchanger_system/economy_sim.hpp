#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <mutex>
#include <chrono>
#include "../../core/sdk/structs.hpp"
#include "inventory_core.hpp"

namespace skinchanger {

// Economy Simulation (fake balance, case/key prices, sell values - visual only)
class EconomySim {
public:
    struct MarketItem {
        int itemDefIndex = 0;
        std::string name;
        uint64_t buyPrice = 0;
        uint64_t sellPrice = 0;
        uint64_t marketPrice = 0; // Steam market approximation
        int rarity = 0;
        float priceTrend = 0.0f; // -1.0 to 1.0
        std::chrono::steady_clock::time_point lastUpdate;
        int volume24h = 0;
        bool isCase = false;
        bool isKey = false;
        bool isSticker = false;
        bool isMusicKit = false;
        bool isAgent = false;
        bool isGlove = false;
        bool isMedal = false;
    };
    
    struct Transaction {
        enum class Type { Buy, Sell, OpenCase, Trade, Gift } type;
        int itemDefIndex = 0;
        int quantity = 1;
        uint64_t price = 0;
        uint64_t total = 0;
        std::chrono::steady_clock::time_point timestamp;
        std::string details;
    };
    
    struct EconomyState {
        uint64_t balance = 1000000; // 1M coins default
        std::vector<Transaction> history;
        size_t maxHistory = 1000;
        bool showPrices = true;
        bool showTrend = true;
        bool autoRefresh = true;
        int refreshInterval = 300; // seconds
    };
    
    static EconomySim& Instance() {
        static EconomySim instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    void Render();
    void Update(float dt);
    
    // Balance
    uint64_t GetBalance() const { return m_state.balance; }
    void SetBalance(uint64_t balance) { m_state.balance = balance; }
    bool AddBalance(uint64_t amount);
    bool SpendBalance(uint64_t amount);
    
    // Buy/Sell
    bool BuyItem(int itemDefIndex, int quantity = 1);
    bool SellItem(int itemDefIndex, int quantity = 1);
    bool CanAfford(int itemDefIndex, int quantity = 1) const;
    
    // Case opening cost
    uint64_t GetCaseOpeningCost(int caseId, int keyId) const;
    bool CanOpenCase(int caseId, int keyId) const;
    
    // Market prices
    std::optional<MarketItem> GetMarketItem(int itemDefIndex) const;
    std::vector<MarketItem> GetAllMarketItems() const;
    std::vector<MarketItem> GetMarketItemsByType(bool isCase, bool isKey, bool isSticker, 
                                                  bool isMusicKit, bool isAgent, bool isGlove, bool isMedal) const;
    std::vector<MarketItem> SearchMarket(const std::string& query) const;
    std::vector<MarketItem> GetTopGainers(int count = 10) const;
    std::vector<MarketItem> GetTopLosers(int count = 10) const;
    std::vector<MarketItem> GetMostTraded(int count = 10) const;
    
    // Price updates
    void RefreshPrices();
    void SimulatePriceChanges();
    
    // History
    const std::vector<Transaction>& GetHistory() const { return m_state.history; }
    void AddTransaction(const Transaction& transaction);
    void ClearHistory();
    
    // State
    EconomyState& GetState() { return m_state; }
    const EconomyState& GetState() const { return m_state; }
    
    // Config
    struct Config {
        bool enabled = true;
        bool showInInventory = true;
        bool simulateMarket = true;
        float priceVolatility = 0.05f; // 5% max change per update
        int maxHistory = 1000;
        uint64_t startingBalance = 1000000;
    };
    
    Config& GetConfig() { return m_config; }
    void LoadConfig();
    void SaveConfig();

private:
    EconomySim() = default;
    EconomyState m_state;
    Config m_config;
    std::unordered_map<int, MarketItem> m_marketItems;
    std::mt19937 m_rng;
    std::mutex m_mutex;
    std::chrono::steady_clock::time_point m_lastPriceUpdate;
    
    void InitializeMarketItems();
    void InitializePrices();
    void GenerateMarketHistory();
    MarketItem CreateMarketItem(int itemDefIndex, const std::string& name, uint64_t basePrice, int rarity, bool isCase, bool isKey);
    void UpdateItemPrice(MarketItem& item);
    uint64_t CalculateSellPrice(uint64_t buyPrice, int rarity) const;
    void RecordTransaction(Transaction::Type type, int itemDefIndex, int quantity, uint64_t price);
};

} // namespace skinchanger