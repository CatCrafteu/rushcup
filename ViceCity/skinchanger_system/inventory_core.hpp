#pragma once

#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../../core/sdk/structs.hpp"
#include "../../core/config/config_manager.hpp"

namespace skinchanger {

using json = nlohmann::json;

// Inventory item structure (memesense parity)
struct InventoryItem {
    uint64_t itemId = 0;              // Unique per item
    int itemDefIndex = 0;             // Weapon/Glove/Agent/Music/Sticker/Case/Key
    int paintKit = 0;                 // Skin ID
    float wear = 0.001f;              // 0.00 - 1.00
    int seed = 0;                     // Pattern seed (0-1000)
    int statTrak = -1;                // -1 = none, >=0 = kills
    char customName[32] = {0};        // Name tag
    struct StickerApplied {
        int stickerId = 0;            // Sticker kit ID
        int slot = 0;                 // 0-3 (5 for gloves)
        float wear = 0.0f;            // Sticker wear 0.0-1.0
        float scale = 1.0f;           // 0.1 - 5.0
        float rotation = 0.0f;        // 0-360
        sdk::Vector2D offset = {0, 0}; // Position offset
        bool isPeeled = false;        // Peel animation state
    } stickers[5];                    // Slot 0-3, 4 for gloves
    int charmId = 0;                  // Charm (CS2)
    int patch = 0;                    // Agent patch
    bool isSouvenir = false;
    bool isTournament = false;
    int tournamentId = 0;
    int tournamentStage = 0;
    int tournamentTeam1 = 0;
    int tournamentTeam2 = 0;
    uint32_t rarity = 0;              // 0=Consumer ... 7=Contraband
    int musicIndex = 0;               // For music kits
    int agentIndex = 0;               // For agents
    int gloveIndex = 0;               // For gloves
    int medalIndex = 0;               // For medals
    
    // Serialization
    json ToJson() const;
    static InventoryItem FromJson(const json& j);
};

// Inventory manager (full client-side simulation)
class InventoryCore {
public:
    static InventoryCore& Instance() {
        static InventoryCore instance;
        return instance;
    }
    
    void Initialize();
    void Shutdown();
    
    // Item management
    uint64_t AddItem(const InventoryItem& item);
    bool RemoveItem(uint64_t itemId);
    std::optional<InventoryItem> GetItem(uint64_t itemId);
    std::vector<InventoryItem> GetItemsByType(int itemDefIndex);
    std::vector<InventoryItem> GetAllItems();
    std::vector<InventoryItem> GetWeapons();
    std::vector<InventoryItem> GetGloves();
    std::vector<InventoryItem> GetAgents();
    std::vector<InventoryItem> GetMusicKits();
    std::vector<InventoryItem> GetStickers();
    std::vector<InventoryItem> GetCases();
    std::vector<InventoryItem> GetKeys();
    std::vector<InventoryItem> GetCharms();
    std::vector<InventoryItem> GetPatches();
    std::vector<InventoryItem> GetMedals();
    
    // Item modification
    bool UpdateItem(uint64_t itemId, const InventoryItem& item);
    bool SetItemWear(uint64_t itemId, float wear);
    bool SetItemSeed(uint64_t itemId, int seed);
    bool SetItemStatTrak(uint64_t itemId, int kills);
    bool SetItemNameTag(uint64_t itemId, const std::string& name);
    bool SetItemSticker(uint64_t itemId, int slot, const InventoryItem::StickerApplied& sticker);
    bool RemoveItemSticker(uint64_t itemId, int slot);
    bool SetItemCharm(uint64_t itemId, int charmId);
    bool SetItemPatch(uint64_t itemId, int patchId);
    
    // Equip/Unequip
    bool EquipItem(uint64_t itemId, int slot = 0);
    bool UnequipItem(int itemDefIndex, int slot = 0);
    std::optional<uint64_t> GetEquippedItem(int itemDefIndex, int slot = 0);
    
    // Persistence
    void SaveToFile(const std::string& path = "ViceCity/inventory.json");
    void LoadFromFile(const std::string& path = "ViceCity/inventory.json");
    void SaveToConfig();
    void LoadFromConfig();
    
    // Economy
    uint64_t GetBalance() const { return m_balance; }
    void SetBalance(uint64_t balance) { m_balance = balance; }
    void AddBalance(uint64_t amount) { m_balance += amount; }
    bool SpendBalance(uint64_t amount);
    
    // Fake prices (visual only)
    struct PriceInfo {
        uint64_t buyPrice = 0;
        uint64_t sellPrice = 0;
        std::string currency = "coins";
    };
    PriceInfo GetPriceInfo(int itemDefIndex) const;
    void SetPriceInfo(int itemDefIndex, const PriceInfo& info);
    
    // Inventory JSON schema
    static constexpr const char* INVENTORY_VERSION = "1.0";
    static constexpr const char* INVENTORY_FILE = "ViceCity/inventory.json";
    
    // Generate unique item ID
    uint64_t GenerateItemId();

private:
    InventoryCore() = default;
    ~InventoryCore() = default;
    
    std::unordered_map<uint64_t, InventoryItem> m_items;
    std::unordered_map<int, std::vector<uint64_t>> m_itemsByType; // itemDefIndex -> itemIds
    std::unordered_map<int, uint64_t> m_equippedItems; // itemDefIndex -> itemId (slot 0)
    uint64_t m_balance = 1000000; // 1M coins default
    std::unordered_map<int, PriceInfo> m_prices;
    uint64_t m_nextItemId = 1;
    std::mutex m_mutex;
    
    void InitializeDefaultInventory();
    void InitializePrices();
    void RebuildTypeIndex();
    void ApplyItemToGame(uint64_t itemId);
    void RemoveItemFromGame(uint64_t itemId);
};

// Inventory JSON schema
inline const json INVENTORY_SCHEMA = R"({
    "version": "1.0",
    "type": "object",
    "properties": {
        "version": {"type": "string"},
        "balance": {"type": "integer", "minimum": 0},
        "nextItemId": {"type": "integer", "minimum": 1},
        "items": {
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                    "itemId": {"type": "integer", "minimum": 0},
                    "itemDefIndex": {"type": "integer"},
                    "paintKit": {"type": "integer"},
                    "wear": {"type": "number", "minimum": 0.0, "maximum": 1.0},
                    "seed": {"type": "integer", "minimum": 0, "maximum": 1000},
                    "statTrak": {"type": "integer", "minimum": -1},
                    "customName": {"type": "string", "maxLength": 31},
                    "stickers": {
                        "type": "array",
                        "maxItems": 5,
                        "items": {
                            "type": "object",
                            "properties": {
                                "stickerId": {"type": "integer"},
                                "slot": {"type": "integer", "minimum": 0, "maximum": 4},
                                "wear": {"type": "number", "minimum": 0.0, "maximum": 1.0},
                                "scale": {"type": "number", "minimum": 0.1, "maximum": 5.0},
                                "rotation": {"type": "number", "minimum": 0.0, "maximum": 360.0},
                                "offsetX": {"type": "number"},
                                "offsetY": {"type": "number"},
                                "isPeeled": {"type": "boolean"}
                            },
                            "required": ["stickerId", "slot"]
                        }
                    },
                    "charmId": {"type": "integer"},
                    "patch": {"type": "integer"},
                    "isSouvenir": {"type": "boolean"},
                    "isTournament": {"type": "boolean"},
                    "tournamentId": {"type": "integer"},
                    "tournamentStage": {"type": "integer"},
                    "tournamentTeam1": {"type": "integer"},
                    "tournamentTeam2": {"type": "integer"},
                    "rarity": {"type": "integer", "minimum": 0, "maximum": 7},
                    "musicIndex": {"type": "integer"},
                    "agentIndex": {"type": "integer"},
                    "gloveIndex": {"type": "integer"},
                    "medalIndex": {"type": "integer"}
                },
                "required": ["itemId", "itemDefIndex"]
            }
        },
        "equipped": {
            "type": "object",
            "additionalProperties": {"type": "integer"}
        },
        "prices": {
            "type": "object",
            "additionalProperties": {
                "type": "object",
                "properties": {
                    "buyPrice": {"type": "integer", "minimum": 0},
                    "sellPrice": {"type": "integer", "minimum": 0},
                    "currency": {"type": "string"}
                }
            }
        }
    },
    "required": ["version", "balance", "nextItemId", "items", "equipped", "prices"]
})"_json;

} // namespace skinchanger