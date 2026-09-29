#include "core/pch.hpp"
#include "skinchanger_system/inventory_core.hpp"
#include "core/config/config_manager.hpp"
#include "core/logger/logger.hpp"

namespace skinchanger {

void InventoryCore::Initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Load from file
    LoadFromFile();
    
    // Initialize default inventory if empty
    if (m_items.empty()) {
        InitializeDefaultInventory();
    }
    
    InitializePrices();
    RebuildTypeIndex();
    
    LOG_INFO(Skinchanger, "InventoryCore initialized with {} items", m_items.size());
}

void InventoryCore::Shutdown() {
    SaveToFile();
    SaveToConfig();
    LOG_INFO(Skinchanger, "InventoryCore shutdown");
}

uint64_t InventoryCore::AddItem(const InventoryItem& item) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    InventoryItem newItem = item;
    newItem.itemId = GenerateItemId();
    
    m_items[newItem.itemId] = newItem;
    m_itemsByType[newItem.itemDefIndex].push_back(newItem.itemId);
    
    // Auto-equip if first of type
    if (m_equippedItems.find(newItem.itemDefIndex) == m_equippedItems.end()) {
        EquipItem(newItem.itemId);
    }
    
    SaveToConfig();
    return newItem.itemId;
}

bool InventoryCore::RemoveItem(uint64_t itemId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return false;
    
    int defIndex = it->second.itemDefIndex;
    
    // Remove from type index
    auto& typeVec = m_itemsByType[defIndex];
    typeVec.erase(std::remove(typeVec.begin(), typeVec.end(), itemId), typeVec.end());
    
    // Unequip if equipped
    if (m_equippedItems[defIndex] == itemId) {
        m_equippedItems.erase(defIndex);
        // Auto-equip next item of same type
        if (!typeVec.empty()) {
            EquipItem(typeVec.front());
        }
    }
    
    m_items.erase(it);
    SaveToConfig();
    return true;
}

std::optional<InventoryItem> InventoryCore::GetItem(uint64_t itemId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return std::nullopt;
    return it->second;
}

std::vector<InventoryItem> InventoryCore::GetItemsByType(int itemDefIndex) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<InventoryItem> result;
    auto it = m_itemsByType.find(itemDefIndex);
    if (it == m_itemsByType.end()) return result;
    
    for (uint64_t id : it->second) {
        auto itemIt = m_items.find(id);
        if (itemIt != m_items.end()) {
            result.push_back(itemIt->second);
        }
    }
    
    return result;
}

std::vector<InventoryItem> InventoryCore::GetAllItems() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<InventoryItem> result;
    result.reserve(m_items.size());
    
    for (auto& [id, item] : m_items) {
        result.push_back(item);
    }
    
    return result;
}

std::vector<InventoryItem> InventoryCore::GetWeapons() {
    // Weapon item definition indices
    static const std::vector<int> weaponIndices = {
        1, 2, 3, 4, 7, 8, 9, 10, 11, 13, 14, 16, 17, 19, 23, 24, 25, 26, 27, 28, 29, 30,
        31, 32, 33, 34, 35, 36, 38, 39, 40, 42, 43, 44, 45, 46, 47, 48, 49, 57, 59, 60,
        61, 63, 64, 68, 69, 70, 72, 74, 75, 76, 78, 80, 81, 82, 83, 84, 85
    };
    
    std::vector<InventoryItem> result;
    for (int idx : weaponIndices) {
        auto items = GetItemsByType(idx);
        result.insert(result.end(), items.begin(), items.end());
    }
    
    return result;
}

std::vector<InventoryItem> InventoryCore::GetGloves() {
    return GetItemsByType(5027); // First glove index
}

std::vector<InventoryItem> InventoryCore::GetAgents() {
    // Agent item definition indices would be added here
    return GetItemsByType(0); // Placeholder
}

std::vector<InventoryItem> InventoryCore::GetMusicKits() {
    return GetItemsByType(0); // Placeholder
}

std::vector<InventoryItem> InventoryCore::GetStickers() {
    return GetItemsByType(0); // Placeholder
}

std::vector<InventoryItem> InventoryCore::GetCases() {
    return GetItemsByType(0); // Placeholder
}

std::vector<InventoryItem> InventoryCore::GetKeys() {
    return GetItemsByType(0); // Placeholder
}

std::vector<InventoryItem> InventoryCore::GetCharms() {
    return GetItemsByType(0); // Placeholder
}

std::vector<InventoryItem> InventoryCore::GetPatches() {
    return GetItemsByType(0); // Placeholder
}

std::vector<InventoryItem> InventoryCore::GetMedals() {
    return GetItemsByType(0); // Placeholder
}

bool InventoryCore::UpdateItem(uint64_t itemId, const InventoryItem& item) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return false;
    
    int oldDefIndex = it->second.itemDefIndex;
    it->second = item;
    item.itemId = itemId; // Preserve ID
    
    // Update type index if def index changed
    if (oldDefIndex != item.itemDefIndex) {
        auto& oldVec = m_itemsByType[oldDefIndex];
        oldVec.erase(std::remove(oldVec.begin(), oldVec.end(), itemId), oldVec.end());
        m_itemsByType[item.itemDefIndex].push_back(itemId);
    }
    
    SaveToConfig();
    return true;
}

bool InventoryCore::SetItemWear(uint64_t itemId, float wear) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return false;
    
    it->second.wear = std::clamp(wear, 0.0f, 1.0f);
    SaveToConfig();
    return true;
}

bool InventoryCore::SetItemSeed(uint64_t itemId, int seed) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return false;
    
    it->second.seed = std::clamp(seed, 0, 1000);
    SaveToConfig();
    return true;
}

bool InventoryCore::SetItemStatTrak(uint64_t itemId, int kills) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return false;
    
    it->second.statTrak = std::max(kills, -1);
    SaveToConfig();
    return true;
}

bool InventoryCore::SetItemNameTag(uint64_t itemId, const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return false;
    
    strncpy_s(it->second.customName, name.c_str(), 31);
    it->second.customName[31] = '\0';
    SaveToConfig();
    return true;
}

bool InventoryCore::SetItemSticker(uint64_t itemId, int slot, const InventoryItem::StickerApplied& sticker) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return false;
    if (slot < 0 || slot >= 5) return false;
    
    it->second.stickers[slot] = sticker;
    SaveToConfig();
    return true;
}

bool InventoryCore::RemoveItemSticker(uint64_t itemId, int slot) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return false;
    if (slot < 0 || slot >= 5) return false;
    
    it->second.stickers[slot] = InventoryItem::StickerApplied{};
    SaveToConfig();
    return true;
}

bool InventoryCore::SetItemCharm(uint64_t itemId, int charmId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return false;
    
    it->second.charmId = charmId;
    SaveToConfig();
    return true;
}

bool InventoryCore::SetItemPatch(uint64_t itemId, int patchId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return false;
    
    it->second.patch = patchId;
    SaveToConfig();
    return true;
}

bool InventoryCore::EquipItem(uint64_t itemId, int slot) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_items.find(itemId);
    if (it == m_items.end()) return false;
    
    // Only support slot 0 for now
    if (slot != 0) return false;
    
    m_equippedItems[it->second.itemDefIndex] = itemId;
    SaveToConfig();
    return true;
}

bool InventoryCore::UnequipItem(int itemDefIndex, int slot) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (slot != 0) return false;
    
    auto it = m_equippedItems.find(itemDefIndex);
    if (it == m_equippedItems.end()) return false;
    
    m_equippedItems.erase(it);
    SaveToConfig();
    return true;
}

std::optional<uint64_t> InventoryCore::GetEquippedItem(int itemDefIndex, int slot) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (slot != 0) return std::nullopt;
    
    auto it = m_equippedItems.find(itemDefIndex);
    if (it == m_equippedItems.end()) return std::nullopt;
    
    return it->second;
}

void InventoryCore::SaveToFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::filesystem::path filePath = path.empty() ? INVENTORY_FILE : path;
    std::filesystem::create_directories(filePath.parent_path());
    
    json j;
    j["version"] = INVENTORY_VERSION;
    j["balance"] = m_balance;
    j["nextItemId"] = m_nextItemId;
    
    // Items
    json itemsArray = json::array();
    for (auto& [id, item] : m_items) {
        itemsArray.push_back(item.ToJson());
    }
    j["items"] = itemsArray;
    
    // Equipped
    json equippedObj = json::object();
    for (auto& [defIndex, itemId] : m_equippedItems) {
        equippedObj[std::to_string(defIndex)] = itemId;
    }
    j["equipped"] = equippedObj;
    
    // Prices
    json pricesObj = json::object();
    for (auto& [defIndex, price] : m_prices) {
        pricesObj[std::to_string(defIndex)] = {
            {"buyPrice", price.buyPrice},
            {"sellPrice", price.sellPrice},
            {"currency", price.currency}
        };
    }
    j["prices"] = pricesObj;
    
    std::ofstream file(filePath);
    if (file.is_open()) {
        file << j.dump(4);
    }
}

void InventoryCore::LoadFromFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::filesystem::path filePath = path.empty() ? INVENTORY_FILE : path;
    if (!std::filesystem::exists(filePath)) return;
    
    try {
        std::ifstream file(filePath);
        json j;
        file >> j;
        
        // Version check
        std::string version = j.value("version", "1.0");
        
        m_balance = j.value("balance", 1000000);
        m_nextItemId = j.value("nextItemId", 1);
        
        // Items
        if (j.contains("items")) {
            for (auto& itemJson : j["items"]) {
                InventoryItem item = InventoryItem::FromJson(itemJson);
                m_items[item.itemId] = item;
                m_itemsByType[item.itemDefIndex].push_back(item.itemId);
            }
        }
        
        // Equipped
        if (j.contains("equipped")) {
            for (auto& [key, value] : j["equipped"].items()) {
                int defIndex = std::stoi(key);
                uint64_t itemId = value.get<uint64_t>();
                m_equippedItems[defIndex] = itemId;
            }
        }
        
        // Prices
        if (j.contains("prices")) {
            for (auto& [key, value] : j["prices"].items()) {
                int defIndex = std::stoi(key);
                m_prices[defIndex] = {
                    value.value("buyPrice", 0),
                    value.value("sellPrice", 0),
                    value.value("currency", "coins")
                };
            }
        }
        
        RebuildTypeIndex();
        LOG_INFO(Skinchanger, "Loaded inventory from file: {} items", m_items.size());
    } catch (const std::exception& e) {
        LOG_ERROR(Skinchanger, "Failed to load inventory: {}", e.what());
    }
}

void InventoryCore::SaveToConfig() {
    auto& config = core::config::ConfigManager::Instance();
    
    json j;
    j["version"] = INVENTORY_VERSION;
    j["balance"] = m_balance;
    j["nextItemId"] = m_nextItemId;
    
    json itemsArray = json::array();
    for (auto& [id, item] : m_items) {
        itemsArray.push_back(item.ToJson());
    }
    j["items"] = itemsArray;
    
    json equippedObj = json::object();
    for (auto& [defIndex, itemId] : m_equippedItems) {
        equippedObj[std::to_string(defIndex)] = itemId;
    }
    j["equipped"] = equippedObj;
    
    config.Set(SKINS_KEY, j);
}

void InventoryCore::LoadFromConfig() {
    auto& config = core::config::ConfigManager::Instance();
    auto opt = config.Get<json>(SKINS_KEY);
    
    if (!opt) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    json j = *opt;
    m_balance = j.value("balance", 1000000);
    m_nextItemId = j.value("nextItemId", 1);
    
    m_items.clear();
    m_itemsByType.clear();
    m_equippedItems.clear();
    
    if (j.contains("items")) {
        for (auto& itemJson : j["items"]) {
            InventoryItem item = InventoryItem::FromJson(itemJson);
            m_items[item.itemId] = item;
            m_itemsByType[item.itemDefIndex].push_back(item.itemId);
        }
    }
    
    if (j.contains("equipped")) {
        for (auto& [key, value] : j["equipped"].items()) {
            int defIndex = std::stoi(key);
            uint64_t itemId = value.get<uint64_t>();
            m_equippedItems[defIndex] = itemId;
        }
    }
    
    RebuildTypeIndex();
}

uint64_t InventoryCore::GetBalance() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_balance;
}

void InventoryCore::SetBalance(uint64_t balance) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_balance = balance;
    SaveToConfig();
}

void InventoryCore::AddBalance(uint64_t amount) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_balance += amount;
    SaveToConfig();
}

bool InventoryCore::SpendBalance(uint64_t amount) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_balance < amount) return false;
    m_balance -= amount;
    SaveToConfig();
    return true;
}

InventoryCore::PriceInfo InventoryCore::GetPriceInfo(int itemDefIndex) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_prices.find(itemDefIndex);
    if (it == m_prices.end()) {
        return {0, 0, "coins"};
    }
    return it->second;
}

void InventoryCore::SetPriceInfo(int itemDefIndex, const PriceInfo& info) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_prices[itemDefIndex] = info;
    SaveToConfig();
}

uint64_t InventoryCore::GenerateItemId() {
    return m_nextItemId++;
}

void InventoryCore::InitializeDefaultInventory() {
    // Add default items for each weapon type
    // This would be populated with actual default skins
    
    // Example: Add a default AK-47
    InventoryItem ak47;
    ak47.itemDefIndex = 7; // Weapon_AK47
    ak47.paintKit = 0; // Default
    ak47.wear = 0.001f;
    ak47.seed = 0;
    ak47.rarity = 0;
    AddItem(ak47);
    
    // Add default gloves
    InventoryItem gloves;
    gloves.itemDefIndex = 5027; // Glove_Studded_Bloodhound
    gloves.paintKit = 0;
    gloves.wear = 0.001f;
    gloves.seed = 0;
    gloves.rarity = 0;
    AddItem(gloves);
    
    LOG_INFO(Skinchanger, "Initialized default inventory");
}

void InventoryCore::InitializePrices() {
    // Base prices for items (visual only)
    // Weapons
    SetPriceInfo(7, {5000, 2500, "coins"}); // AK-47
    SetPriceInfo(9, {10000, 5000, "coins"}); // AWP
    SetPriceInfo(16, {4500, 2250, "coins"}); // M4A4
    SetPriceInfo(60, {5500, 2750, "coins"}); // M4A1-S
    SetPriceInfo(1, {700, 350, "coins"}); // Deagle
    SetPriceInfo(4, {200, 100, "coins"}); // Glock
    SetPriceInfo(3, {500, 250, "coins"}); // Five-Seven
    SetPriceInfo(32, {200, 100, "coins"}); // USP-S
    SetPriceInfo(61, {200, 100, "coins"}); // P2000
    SetPriceInfo(63, {500, 250, "coins"}); // CZ75
    SetPriceInfo(64, {850, 425, "coins"}); // Revolver
    
    // Knives (expensive)
    SetPriceInfo(500, {100000, 50000, "coins"}); // Bayonet
    SetPriceInfo(505, {80000, 40000, "coins"}); // Flip
    SetPriceInfo(506, {70000, 35000, "coins"}); // Gut
    SetPriceInfo(507, {120000, 60000, "coins"}); // Karambit
    SetPriceInfo(508, {90000, 45000, "coins"}); // M9 Bayonet
    
    // Gloves
    SetPriceInfo(5027, {50000, 25000, "coins"}); // Bloodhound
    SetPriceInfo(5028, {30000, 15000, "coins"}); // T-side
    SetPriceInfo(5029, {30000, 15000, "coins"}); // CT-side
    SetPriceInfo(5030, {40000, 20000, "coins"}); // Sporty
    SetPriceInfo(5031, {45000, 22500, "coins"}); // Slick
    SetPriceInfo(5032, {40000, 20000, "coins"}); // Leather
    SetPriceInfo(5033, {50000, 25000, "coins"}); // Motorcycle
    SetPriceInfo(5034, {60000, 30000, "coins"}); // Specialist
    SetPriceInfo(5035, {70000, 35000, "coins"}); // Hydra
    
    // Cases
    SetPriceInfo(0, {1000, 500, "coins"}); // Placeholder for cases
    
    // Keys
    SetPriceInfo(0, {2500, 1250, "coins"}); // Placeholder for keys
    
    // Stickers
    SetPriceInfo(0, {500, 250, "coins"}); // Placeholder for stickers
}

void InventoryCore::RebuildTypeIndex() {
    m_itemsByType.clear();
    for (auto& [id, item] : m_items) {
        m_itemsByType[item.itemDefIndex].push_back(id);
    }
}

void InventoryCore::ApplyItemToGame(uint64_t itemId) {
    // Apply skin to game entity
    // This would hook into the game's skin system
}

void InventoryCore::RemoveItemFromGame(uint64_t itemId) {
    // Remove skin from game entity
}

json InventoryItem::ToJson() const {
    json j;
    j["itemId"] = itemId;
    j["itemDefIndex"] = itemDefIndex;
    j["paintKit"] = paintKit;
    j["wear"] = wear;
    j["seed"] = seed;
    j["statTrak"] = statTrak;
    j["customName"] = customName;
    
    json stickersArray = json::array();
    for (int i = 0; i < 5; ++i) {
        const auto& s = stickers[i];
        if (s.stickerId != 0) {
            stickersArray.push_back({
                {"stickerId", s.stickerId},
                {"slot", s.slot},
                {"wear", s.wear},
                {"scale", s.scale},
                {"rotation", s.rotation},
                {"offsetX", s.offset.x},
                {"offsetY", s.offset.y},
                {"isPeeled", s.isPeeled}
            });
        }
    }
    j["stickers"] = stickersArray;
    
    j["charmId"] = charmId;
    j["patch"] = patch;
    j["isSouvenir"] = isSouvenir;
    j["isTournament"] = isTournament;
    j["tournamentId"] = tournamentId;
    j["tournamentStage"] = tournamentStage;
    j["tournamentTeam1"] = tournamentTeam1;
    j["tournamentTeam2"] = tournamentTeam2;
    j["rarity"] = rarity;
    j["musicIndex"] = musicIndex;
    j["agentIndex"] = agentIndex;
    j["gloveIndex"] = gloveIndex;
    j["medalIndex"] = medalIndex;
    
    return j;
}

InventoryItem InventoryItem::FromJson(const json& j) {
    InventoryItem item;
    item.itemId = j.value("itemId", 0);
    item.itemDefIndex = j.value("itemDefIndex", 0);
    item.paintKit = j.value("paintKit", 0);
    item.wear = j.value("wear", 0.001f);
    item.seed = j.value("seed", 0);
    item.statTrak = j.value("statTrak", -1);
    
    std::string name = j.value("customName", "");
    strncpy_s(item.customName, name.c_str(), 31);
    item.customName[31] = '\0';
    
    if (j.contains("stickers")) {
        for (auto& s : j["stickers"]) {
            int slot = s.value("slot", 0);
            if (slot >= 0 && slot < 5) {
                item.stickers[slot].stickerId = s.value("stickerId", 0);
                item.stickers[slot].slot = slot;
                item.stickers[slot].wear = s.value("wear", 0.0f);
                item.stickers[slot].scale = s.value("scale", 1.0f);
                item.stickers[slot].rotation = s.value("rotation", 0.0f);
                item.stickers[slot].offset.x = s.value("offsetX", 0.0f);
                item.stickers[slot].offset.y = s.value("offsetY", 0.0f);
                item.stickers[slot].isPeeled = s.value("isPeeled", false);
            }
        }
    }
    
    item.charmId = j.value("charmId", 0);
    item.patch = j.value("patch", 0);
    item.isSouvenir = j.value("isSouvenir", false);
    item.isTournament = j.value("isTournament", false);
    item.tournamentId = j.value("tournamentId", 0);
    item.tournamentStage = j.value("tournamentStage", 0);
    item.tournamentTeam1 = j.value("tournamentTeam1", 0);
    item.tournamentTeam2 = j.value("tournamentTeam2", 0);
    item.rarity = j.value("rarity", 0);
    item.musicIndex = j.value("musicIndex", 0);
    item.agentIndex = j.value("agentIndex", 0);
    item.gloveIndex = j.value("gloveIndex", 0);
    item.medalIndex = j.value("medalIndex", 0);
    
    return item;
}

} // namespace skinchanger