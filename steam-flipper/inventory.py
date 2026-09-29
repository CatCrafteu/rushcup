from typing import List, Dict, Set
import steam_client
import database
import config

def scan_inventory(app_id: int = 730) -> List[Dict]:
    """Scan full inventory and return marketable items."""
    print(f"[Inventory] Scanning inventory for app {app_id}...")
    items = steam_client.steam_client.get_inventory(app_id)
    marketable = [item.__dict__ for item in items if item.marketable and item.tradable]
    print(f"[Inventory] Found {len(marketable)} marketable items")
    return marketable

def identify_used_items(inventory: List[Dict]) -> Set[str]:
    """
    Identify items currently equipped/used by the owner.
    This is a heuristic - checks for items in 'active' loadout slots.
    User should manually mark their used items in config or we detect via GC.
    """
    used = set()
    
    # Method 1: Check config for manually specified asset IDs
    manual_used = config.config.get("manual_used_asset_ids", [])
    used.update(manual_used)
    
    # Method 2: Items with certain tags (equipped, favorite, etc.)
    # This would require GC connection - skipping for now
    
    # Method 3: Items not in marketable inventory but in backpack
    # Already filtered by marketable=True above
    
    print(f"[Inventory] Identified {len(used)} used/protected asset IDs")
    return used

def get_flippable_items(inventory: List[Dict], used_asset_ids: Set[str]) -> List[Dict]:
    """Filter inventory to only items safe to flip (not used by owner)."""
    flippable = []
    for item in inventory:
        asset_id = item.get("asset_id") or f"{item['class_id']}_{item['instance_id']}"
        if asset_id not in used_asset_ids:
            flippable.append(item)
    
    print(f"[Inventory] {len(flippable)} items safe to flip (excluding {len(used_asset_ids)} used)")
    return flippable

def get_inventory_value(inventory: List[Dict]) -> float:
    """Estimate total inventory value from current market prices."""
    total = 0.0
    for item in inventory:
        price_data = steam_client.steam_client.get_price_overview(item["market_hash_name"], item["app_id"])
        if price_data and "median_price" in price_data:
            price_str = price_data["median_price"].replace("$", "").replace(",", "")
            try:
                total += float(price_str) * item.get("amount", 1)
            except:
                pass
    return total

def sync_inventory_to_db():
    """Full inventory sync - scans, identifies used items, saves to DB."""
    inventory = scan_inventory()
    used_ids = identify_used_items(inventory)
    database.save_inventory_snapshot(inventory, used_ids)
    return inventory, used_ids

def can_sell_item(market_hash_name: str, asset_id: str = None) -> bool:
    """Check if an item is safe to sell (not used by owner)."""
    used_ids = database.get_used_asset_ids()
    if asset_id and asset_id in used_ids:
        return False
    return not database.is_item_used_by_owner(market_hash_name, used_ids)