#include "core/pch.hpp"
#include "skinchanger_system/economy_sim.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "core/logger/logger.hpp"
#include "core/config/config_manager.hpp"

namespace skinchanger {

void EconomySim::Initialize() {
    InitializeMarketItems();
    InitializePrices();
    GenerateMarketHistory();
    LoadConfig();
    LOG_INFO(Skinchanger, "EconomySim initialized");
}

void EconomySim::Shutdown() {
    SaveConfig();
    LOG_INFO(Skinchanger, "EconomySim shutdown");
}

void EconomySim::Render() {
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Economy Simulator")) {
        // Balance
        ImGui::Text("Balance: %llu coins", m_state.balance);
        ImGui::SameLine();
        if (ImGui::Button("Add 10000")) AddBalance(10000);
        ImGui::SameLine();
        if (ImGui::Button("Reset")) SetBalance(m_config.startingBalance);
        ImGui::Separator();
        
        // Tabs
        if (ImGui::BeginTabBar("##economy_tabs")) {
            if (ImGui::BeginTabItem("Market")) {
                RenderMarketTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Top Gainers")) {
                RenderTopGainersTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Top Losers")) {
                RenderTopLosersTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Most Traded")) {
                RenderMostTradedTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("History")) {
                RenderHistoryTab();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void EconomySim::Update(float dt) {
    if (m_config.simulateMarket) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastPriceUpdate).count();
        
        if (elapsed >= m_state.refreshInterval) {
            SimulatePriceChanges();
            m_lastPriceUpdate = now;
        }
    }
}

bool EconomySim::AddBalance(uint64_t amount) {
    m_state.balance += amount;
    return true;
}

bool EconomySim::SpendBalance(uint64_t amount) {
    if (m_state.balance < amount) return false;
    m_state.balance -= amount;
    return true;
}

bool EconomySim::BuyItem(int itemDefIndex, int quantity) {
    auto item = GetMarketItem(itemDefIndex);
    if (!item) return false;
    
    uint64_t total = item->buyPrice * quantity;
    if (!SpendBalance(total)) return false;
    
    // Add to inventory
    auto& inv = InventoryCore::Instance();
    for (int i = 0; i < quantity; ++i) {
        InventoryItem newItem;
        newItem.itemDefIndex = itemDefIndex;
        newItem.paintKit = 0;
        newItem.wear = 0.001f;
        newItem.seed = 0;
        newItem.statTrak = -1;
        newItem.rarity = item->rarity;
        inv.AddItem(newItem);
    }
    
    RecordTransaction(Transaction::Type::Buy, itemDefIndex, quantity, item->buyPrice);
    return true;
}

bool EconomySim::SellItem(int itemDefIndex, int quantity) {
    auto item = GetMarketItem(itemDefIndex);
    if (!item) return false;
    
    // Check inventory
    auto& inv = InventoryCore::Instance();
    auto items = inv.GetItemsByType(itemDefIndex);
    if ((int)items.size() < quantity) return false;
    
    uint64_t total = item->sellPrice * quantity;
    m_state.balance += total;
    
    // Remove from inventory
    for (int i = 0; i < quantity; ++i) {
        inv.RemoveItem(items[i].itemId);
    }
    
    RecordTransaction(Transaction::Type::Sell, itemDefIndex, quantity, item->sellPrice);
    return true;
}

bool EconomySim::CanAfford(int itemDefIndex, int quantity) const {
    auto item = GetMarketItem(itemDefIndex);
    if (!item) return false;
    return m_state.balance >= item->buyPrice * quantity;
}

uint64_t EconomySim::GetCaseOpeningCost(int caseId, int keyId) const {
    auto caseIt = std::find_if(m_marketItems.begin(), m_marketItems.end(),
        [caseId](const auto& p) { return p.first == caseId && p.second.isCase; });
    auto keyIt = std::find_if(m_marketItems.begin(), m_marketItems.end(),
        [keyId](const auto& p) { return p.first == keyId && p.second.isKey; });
    
    if (caseIt == m_marketItems.end() || keyIt == m_marketItems.end()) return 0;
    return caseIt->second.buyPrice + keyIt->second.buyPrice;
}

bool EconomySim::CanOpenCase(int caseId, int keyId) const {
    return CanAfford(caseId, 1) && CanAfford(keyId, 1);
}

std::optional<EconomySim::MarketItem> EconomySim::GetMarketItem(int itemDefIndex) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_marketItems.find(itemDefIndex);
    if (it == m_marketItems.end()) return std::nullopt;
    return it->second;
}

std::vector<EconomySim::MarketItem> EconomySim::GetAllMarketItems() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<MarketItem> result;
    result.reserve(m_marketItems.size());
    for (auto& [_, item] : m_marketItems) result.push_back(item);
    return result;
}

std::vector<EconomySim::MarketItem> EconomySim::GetMarketItemsByType(bool isCase, bool isKey, bool isSticker, 
                                                                     bool isMusicKit, bool isAgent, bool isGlove, bool isMedal) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<MarketItem> result;
    for (auto& [_, item] : m_marketItems) {
        if ((isCase && item.isCase) || (isKey && item.isKey) || (isSticker && item.isSticker) ||
            (isMusicKit && item.isMusicKit) || (isAgent && item.isAgent) || (isGlove && item.isGlove) ||
            (isMedal && item.isMedal)) {
            result.push_back(item);
        }
    }
    return result;
}

std::vector<EconomySim::MarketItem> EconomySim::SearchMarket(const std::string& query) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<MarketItem> result;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (auto& [_, item] : m_marketItems) {
        std::string lowerName = item.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        if (lowerName.find(lowerQuery) != std::string::npos) {
            result.push_back(item);
        }
    }
    return result;
}

std::vector<EconomySim::MarketItem> EconomySim::GetTopGainers(int count) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<MarketItem> items;
    for (auto& [_, item] : m_marketItems) items.push_back(item);
    
    std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
        return a.priceTrend > b.priceTrend;
    });
    
    if ((int)items.size() > count) items.resize(count);
    return items;
}

std::vector<EconomySim::MarketItem> EconomySim::GetTopLosers(int count) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<MarketItem> items;
    for (auto& [_, item] : m_marketItems) items.push_back(item);
    
    std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
        return a.priceTrend < b.priceTrend;
    });
    
    if ((int)items.size() > count) items.resize(count);
    return items;
}

std::vector<EconomySim::MarketItem> EconomySim::GetMostTraded(int count) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<MarketItem> items;
    for (auto& [_, item] : m_marketItems) items.push_back(item);
    
    std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
        return a.volume24h > b.volume24h;
    });
    
    if ((int)items.size() > count) items.resize(count);
    return items;
}

void EconomySim::RefreshPrices() {
    SimulatePriceChanges();
}

void EconomySim::SimulatePriceChanges() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& [_, item] : m_marketItems) {
        UpdateItemPrice(item);
    }
}

const std::vector<EconomySim::Transaction>& EconomySim::GetHistory() const {
    return m_state.history;
}

void EconomySim::AddTransaction(const Transaction& transaction) {
    m_state.history.push_back(transaction);
    if (m_state.history.size() > m_state.maxHistory) {
        m_state.history.erase(m_state.history.begin());
    }
}

void EconomySim::ClearHistory() {
    m_state.history.clear();
}

EconomySim::Config& EconomySim::GetConfig() {
    return m_config;
}

void EconomySim::LoadConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    auto opt = config.Get<json>("skinchanger_economy", "economy");
    if (opt) {
        try {
            m_config.enabled = opt->value("enabled", true);
            m_config.showInInventory = opt->value("showInInventory", true);
            m_config.simulateMarket = opt->value("simulateMarket", true);
            m_config.priceVolatility = opt->value("priceVolatility", 0.05f);
            m_config.maxHistory = opt->value("maxHistory", 1000);
            m_config.startingBalance = opt->value("startingBalance", 1000000);
            
            m_state.maxHistory = m_config.maxHistory;
            m_state.balance = m_config.startingBalance;
        } catch (...) {}
    }
}

void EconomySim::SaveConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    json j;
    j["enabled"] = m_config.enabled;
    j["showInInventory"] = m_config.showInInventory;
    j["simulateMarket"] = m_config.simulateMarket;
    j["priceVolatility"] = m_config.priceVolatility;
    j["maxHistory"] = m_config.maxHistory;
    j["startingBalance"] = m_config.startingBalance;
    
    config.Set("skinchanger_economy", "economy", j);
}

void EconomySim::RenderMarketTab() {
    // Search
    static std::string searchQuery;
    ImGui::InputText("Search", &searchQuery);
    ImGui::SameLine();
    ImGui::Checkbox("Show Prices", &m_state.showPrices);
    ImGui::SameLine();
    ImGui::Checkbox("Show Trend", &m_state.showTrend);
    ImGui::Separator();
    
    // Filter tabs
    if (ImGui::BeginTabBar("##market_filter")) {
        if (ImGui::BeginTabItem("All")) { RenderMarketItems(GetAllMarketItems()); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Weapons")) { RenderMarketItems(GetMarketItemsByType(false, false, false, false, false, false, false)); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Cases")) { RenderMarketItems(GetMarketItemsByType(true, false, false, false, false, false, false)); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Keys")) { RenderMarketItems(GetMarketItemsByType(false, true, false, false, false, false, false)); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Stickers")) { RenderMarketItems(GetMarketItemsByType(false, false, true, false, false, false, false)); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Music Kits")) { RenderMarketItems(GetMarketItemsByType(false, false, false, true, false, false, false)); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Agents")) { RenderMarketItems(GetMarketItemsByType(false, false, false, false, true, false, false)); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Gloves")) { RenderMarketItems(GetMarketItemsByType(false, false, false, false, false, true, false)); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Medals")) { RenderMarketItems(GetMarketItemsByType(false, false, false, false, false, false, true)); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
}

void EconomySim::RenderMarketItems(const std::vector<MarketItem>& items) {
    ImGui::BeginChild("##market_list", ImVec2(0, 0), true);
    
    ImGui::Columns(6, "##market_cols", true);
    ImGui::SetColumnWidth(0, 200); // Name
    ImGui::SetColumnWidth(1, 80);  // Rarity
    ImGui::SetColumnWidth(2, 100); // Buy Price
    ImGui::SetColumnWidth(3, 100); // Sell Price
    ImGui::SetColumnWidth(4, 80);  // Trend
    ImGui::SetColumnWidth(5, 100); // Volume
    
    ImGui::Text("Name"); ImGui::NextColumn();
    ImGui::Text("Rarity"); ImGui::NextColumn();
    ImGui::Text("Buy"); ImGui::NextColumn();
    ImGui::Text("Sell"); ImGui::NextColumn();
    ImGui::Text("Trend"); ImGui::NextColumn();
    ImGui::Text("Vol 24h"); ImGui::NextColumn();
    ImGui::Separator();
    
    std::string lowerQuery;
    if (!searchQuery.empty()) {
        lowerQuery = searchQuery;
        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    }
    
    for (const auto& item : items) {
        if (!lowerQuery.empty()) {
            std::string lowerName = item.name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            if (lowerName.find(lowerQuery) == std::string::npos) continue;
        }
        
        // Rarity color
        ImU32 rarityColor = RarityGlow::RARITY_COLORS[item.rarity].nameColor;
        
        ImGui::TextColored(ImColor(rarityColor), "%s", item.name.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", RarityGlow::RARITY_COLORS[item.rarity].name); ImGui::NextColumn();
        
        if (m_state.showPrices) {
            ImGui::Text("%llu", item.buyPrice); ImGui::NextColumn();
            ImGui::Text("%llu", item.sellPrice); ImGui::NextColumn();
        } else {
            ImGui::Text("-"); ImGui::NextColumn();
            ImGui::Text("-"); ImGui::NextColumn();
        }
        
        if (m_state.showTrend) {
            ImU32 trendColor = item.priceTrend > 0 ? IM_COL32(0, 255, 0, 255) : 
                               item.priceTrend < 0 ? IM_COL32(255, 0, 0, 255) : IM_COL32(255, 255, 255, 255);
            ImGui::TextColored(ImColor(trendColor), "%+.2f%%", item.priceTrend * 100); ImGui::NextColumn();
        } else {
            ImGui::Text("-"); ImGui::NextColumn();
        }
        
        ImGui::Text("%d", item.volume24h); ImGui::NextColumn();
        
        // Buy/Sell buttons
        if (ImGui::Button(fmt::format("Buy##{}", item.itemDefIndex).c_str())) {
            BuyItem(item.itemDefIndex, 1);
        }
        ImGui::SameLine();
        if (ImGui::Button(fmt::format("Sell##{}", item.itemDefIndex).c_str())) {
            SellItem(item.itemDefIndex, 1);
        }
    }
    
    ImGui::Columns(1);
    ImGui::EndChild();
}

void EconomySim::RenderTopGainersTab() {
    auto gainers = GetTopGainers(20);
    RenderMarketItems(gainers);
}

void EconomySim::RenderTopLosersTab() {
    auto losers = GetTopLosers(20);
    RenderMarketItems(losers);
}

void EconomySim::RenderMostTradedTab() {
    auto traded = GetMostTraded(20);
    RenderMarketItems(traded);
}

void EconomySim::RenderHistoryTab() {
    ImGui::BeginChild("##history", ImVec2(0, 0), true);
    
    ImGui::Columns(5, "##history_cols", true);
    ImGui::Text("Time"); ImGui::NextColumn();
    ImGui::Text("Type"); ImGui::NextColumn();
    ImGui::Text("Item"); ImGui::NextColumn();
    ImGui::Text("Qty"); ImGui::NextColumn();
    ImGui::Text("Price"); ImGui::NextColumn();
    ImGui::Separator();
    
    for (auto it = m_state.history.rbegin(); it != m_state.history.rend(); ++it) {
        const auto& t = *it;
        auto time = std::chrono::system_clock::to_time_t(t.timestamp);
        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&time));
        
        ImGui::Text("%s", timeStr); ImGui::NextColumn();
        
        const char* typeStr = "Unknown";
        ImU32 typeColor = IM_COL32(255, 255, 255, 255);
        switch (t.type) {
            case Transaction::Type::Buy: typeStr = "BUY"; typeColor = IM_COL32(0, 255, 0, 255); break;
            case Transaction::Type::Sell: typeStr = "SELL"; typeColor = IM_COL32(255, 0, 0, 255); break;
            case Transaction::Type::OpenCase: typeStr = "CASE"; typeColor = IM_COL32(255, 215, 0, 255); break;
            case Transaction::Type::Trade: typeStr = "TRADE"; typeColor = IM_COL32(0, 200, 255, 255); break;
            case Transaction::Type::Gift: typeStr = "GIFT"; typeColor = IM_COL32(255, 0, 255, 255); break;
        }
        ImGui::TextColored(ImColor(typeColor), "%s", typeStr); ImGui::NextColumn();
        ImGui::Text("%s", GetItemName(t.itemDefIndex).c_str()); ImGui::NextColumn();
        ImGui::Text("%d", t.quantity); ImGui::NextColumn();
        ImGui::Text("%llu", t.price); ImGui::NextColumn();
    }
    
    ImGui::Columns(1);
    ImGui::EndChild();
}

void EconomySim::InitializeMarketItems() {
    // Weapons
    CreateMarketItem(7, "AK-47", 2500, 2, false, false);
    CreateMarketItem(9, "AWP", 5000, 5, false, false);
    CreateMarketItem(16, "M4A4", 2200, 3, false, false);
    CreateMarketItem(60, "M4A1-S", 2700, 3, false, false);
    CreateMarketItem(1, "Desert Eagle", 350, 1, false, false);
    CreateMarketItem(4, "Glock-18", 100, 0, false, false);
    CreateMarketItem(3, "Five-SeveN", 250, 1, false, false);
    CreateMarketItem(32, "P2000", 100, 0, false, false);
    CreateMarketItem(61, "USP-S", 100, 0, false, false);
    CreateMarketItem(63, "CZ75-Auto", 250, 1, false, false);
    CreateMarketItem(64, "R8 Revolver", 425, 2, false, false);
    
    // Knives
    CreateMarketItem(500, "Bayonet", 50000, 6, false, false);
    CreateMarketItem(505, "Flip Knife", 40000, 5, false, false);
    CreateMarketItem(506, "Gut Knife", 35000, 4, false, false);
    CreateMarketItem(507, "Karambit", 60000, 6, false, false);
    CreateMarketItem(508, "M9 Bayonet", 45000, 5, false, false);
    CreateMarketItem(509, "Huntsman Knife", 30000, 4, false, false);
    CreateMarketItem(512, "Falchion Knife", 25000, 3, false, false);
    CreateMarketItem(514, "Bowie Knife", 20000, 3, false, false);
    CreateMarketItem(515, "Butterfly Knife", 55000, 6, false, false);
    CreateMarketItem(516, "Shadow Daggers", 15000, 2, false, false);
    
    // Gloves
    CreateMarketItem(5027, "Bloodhound Gloves", 50000, 6, false, false);
    CreateMarketItem(5028, "T-Side Gloves", 30000, 3, false, false);
    CreateMarketItem(5029, "CT-Side Gloves", 30000, 3, false, false);
    CreateMarketItem(5030, "Sporty Gloves", 40000, 4, false, false);
    CreateMarketItem(5031, "Slick Gloves", 45000, 4, false, false);
    CreateMarketItem(5032, "Leather Wrap Gloves", 40000, 3, false, false);
    CreateMarketItem(5033, "Motorcycle Gloves", 50000, 5, false, false);
    CreateMarketItem(5034, "Specialist Gloves", 60000, 6, false, false);
    CreateMarketItem(5035, "Hydra Gloves", 70000, 6, false, false);
    
    // Cases
    for (int i = 1; i <= 20; ++i) {
        CreateMarketItem(1000 + i, fmt::format("Case #{}", i), 1000, 0, true, false);
    }
    
    // Keys
    for (int i = 1; i <= 20; ++i) {
        CreateMarketItem(2000 + i, fmt::format("Key #{}", i), 2500, 0, false, true);
    }
    
    // Stickers
    for (int i = 1; i <= 100; ++i) {
        CreateMarketItem(3000 + i, fmt::format("Sticker #{}", i), 500, 1, false, false);
        m_marketItems[3000 + i].isSticker = true;
    }
    
    // Music Kits
    for (int i = 1; i <= 12; ++i) {
        CreateMarketItem(4000 + i, fmt::format("Music Kit #{}", i), 5000, 3, false, false);
        m_marketItems[4000 + i].isMusicKit = true;
    }
    
    // Agents
    for (int i = 1; i <= 10; ++i) {
        CreateMarketItem(5000 + i, fmt::format("Agent #{}", i), 10000, 2, false, false);
        m_marketItems[5000 + i].isAgent = true;
    }
    
    // Medals
    for (int i = 1; i <= 24; ++i) {
        CreateMarketItem(6000 + i, fmt::format("Medal #{}", i), 10000, 3, false, false);
        m_marketItems[6000 + i].isMedal = true;
    }
}

void EconomySim::InitializePrices() {
    // Prices already set in CreateMarketItem
    // Sell prices calculated
}

EconomySim::MarketItem EconomySim::CreateMarketItem(int itemDefIndex, const std::string& name, uint64_t basePrice, int rarity, bool isCase, bool isKey) {
    MarketItem item;
    item.itemDefIndex = itemDefIndex;
    item.name = name;
    item.buyPrice = basePrice;
    item.sellPrice = CalculateSellPrice(basePrice, rarity);
    item.marketPrice = basePrice;
    item.rarity = rarity;
    item.priceTrend = 0.0f;
    item.lastUpdate = std::chrono::steady_clock::now();
    item.volume24h = rand() % 1000;
    item.isCase = isCase;
    item.isKey = isKey;
    
    m_marketItems[itemDefIndex] = item;
    return item;
}

void EconomySim::GenerateMarketHistory() {
    // Generate fake history
    for (int i = 0; i < 50; ++i) {
        Transaction t;
        t.type = (rand() % 2 == 0) ? Transaction::Type::Buy : Transaction::Type::Sell;
        t.itemDefIndex = 7 + (rand() % 20);
        t.quantity = 1 + (rand() % 3);
        t.price = 1000 + (rand() % 5000);
        t.total = t.price * t.quantity;
        t.timestamp = std::chrono::system_clock::now() - std::chrono::hours(rand() % 168); // Last week
        t.details = "";
        m_state.history.push_back(t);
    }
}

void EconomySim::UpdateItemPrice(MarketItem& item) {
    // Random walk with mean reversion
    float change = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * m_config.priceVolatility;
    float meanReversion = (1.0f - item.priceTrend) * 0.1f;
    change += meanReversion;
    
    item.priceTrend = std::clamp(item.priceTrend + change, -0.2f, 0.2f);
    
    float multiplier = 1.0f + item.priceTrend;
    item.buyPrice = static_cast<uint64_t>(item.buyPrice * multiplier);
    item.sellPrice = CalculateSellPrice(item.buyPrice, item.rarity);
    item.marketPrice = item.buyPrice;
    item.volume24h = rand() % 1000;
    item.lastUpdate = std::chrono::steady_clock::now();
}

uint64_t EconomySim::CalculateSellPrice(uint64_t buyPrice, int rarity) const {
    // Sell price is typically 50-80% of buy price depending on rarity
    float multiplier;
    switch (rarity) {
        case 0: multiplier = 0.50f; break; // Consumer
        case 1: multiplier = 0.55f; break; // Industrial
        case 2: multiplier = 0.60f; break; // Mil-Spec
        case 3: multiplier = 0.65f; break; // Restricted
        case 4: multiplier = 0.70f; break; // Classified
        case 5: multiplier = 0.75f; break; // Covert
        case 6: multiplier = 0.80f; break; // Contraband
        default: multiplier = 0.50f; break;
    }
    return static_cast<uint64_t>(buyPrice * multiplier);
}

void EconomySim::RecordTransaction(Transaction::Type type, int itemDefIndex, int quantity, uint64_t price) {
    Transaction t;
    t.type = type;
    t.itemDefIndex = itemDefIndex;
    t.quantity = quantity;
    t.price = price;
    t.total = price * quantity;
    t.timestamp = std::chrono::system_clock::now();
    t.details = "";
    
    AddTransaction(t);
}

std::string EconomySim::GetItemName(int itemDefIndex) const {
    auto item = GetMarketItem(itemDefIndex);
    if (item) return item->name;
    return fmt::format("Item #{}", itemDefIndex);
}

} // namespace skinchanger