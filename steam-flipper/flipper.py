import time
import signal
import sys
from typing import List, Dict, Optional
from colorama import Fore, Style, init

import config
import database
import steam_client
import inventory
import analyzer
import telegram_bot

init(autoreset=True)

class SteamFlipper:
    def __init__(self):
        self.running = False
        self.steam = steam_client.steam_client
        self.notifier = telegram_bot.notifier
        self.scan_interval = config.config["settings"]["scan_interval_seconds"]
        self.max_daily_spend = config.config["settings"]["max_daily_spend"]
        self.max_listings = config.config["settings"]["max_concurrent_listings"]
        self.buy_duration = config.config["settings"]["buy_order_duration_days"]
        self.undercut = config.config["settings"]["sell_price_undercut_percent"]
        
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
    
    def _signal_handler(self, sig, frame):
        print(f"\n{Fore.YELLOW}[Flipper] Shutdown signal received...{Style.RESET_ALL}")
        self.running = False
    
    def start(self):
        print(f"{Fore.CYAN}{'='*50}")
        print(f"  STEAM MARKET FLIPPER v1.0")
        print(f"  Conservative mode | Main account safe")
        print(f"{'='*50}{Style.RESET_ALL}")
        
        if not self.steam.login():
            print(f"{Fore.RED}[Flipper] Login failed. Check config.json{Style.RESET_ALL}")
            return
        
        self.notifier.notify_startup()
        self.running = True
        
        # Initial inventory sync
        print(f"{Fore.CYAN}[Flipper] Syncing inventory...{Style.RESET_ALL}")
        inventory.sync_inventory_to_db()
        
        # Main loop
        while self.running:
            try:
                self._run_cycle()
            except Exception as e:
                print(f"{Fore.RED}[Flipper] Cycle error: {e}{Style.RESET_ALL}")
                self.notifier.notify_error("main_cycle", str(e))
            
            if self.running:
                print(f"{Fore.CYAN}[Flipper] Sleeping {self.scan_interval}s...{Style.RESET_ALL}")
                for _ in range(self.scan_interval):
                    if not self.running:
                        break
                    time.sleep(1)
        
        self.notifier.notify_shutdown()
        print(f"{Fore.GREEN}[Flipper] Stopped cleanly{Style.RESET_ALL}")
    
    def _run_cycle(self):
        print(f"\n{Fore.CYAN}[Flipper] === New Scan Cycle === {Style.RESET_ALL}")
        
        # Check limits
        daily_spend = database.get_daily_spend()
        active_listings = database.get_active_listings_count()
        
        print(f"[Flipper] Daily spend: ${daily_spend:.2f}/${self.max_daily_spend:.2f} | Active listings: {active_listings}/{self.max_listings}")
        
        if daily_spend >= self.max_daily_spend:
            print(f"{Fore.YELLOW}[Flipper] Daily spend limit reached{Style.RESET_ALL}")
            return
        
        if active_listings >= self.max_listings:
            print(f"{Fore.YELLOW}[Flipper] Max concurrent listings reached{Style.RESET_ALL}")
            self._manage_existing_listings()
            return
        
        # Scan for profitable items
        profitable_items = self._scan_market()
        
        if not profitable_items:
            print(f"{Fore.YELLOW}[Flipper] No profitable items found this cycle{Style.RESET_ALL}")
            return
        
        # Process top opportunities
        for item in profitable_items[:5]:  # Top 5 per cycle
            if not self.running:
                break
            self._process_flip(item)
            
            # Re-check limits after each flip
            daily_spend = database.get_daily_spend()
            if daily_spend >= self.max_daily_spend:
                break
    
    def _scan_market(self) -> List[Dict]:
        """Scan market for profitable flips. Uses known item lists + price checks."""
        # For MVP: check a curated list of high-volume cases/items
        # In production: use steam webapi to get market search results
        
        target_items = self._get_target_items()
        results = []
        
        print(f"[Flipper] Checking {len(target_items)} target items...")
        
        for item_name in target_items:
            if not self.running:
                break
            
            price_data = self.steam.get_price_overview(item_name)
            if not price_data:
                continue
            
            # Parse price overview
            buy_price = self._parse_price(price_data.get("lowest_price"))
            sell_price = self._parse_price(price_data.get("median_price"))
            volume = price_data.get("volume", 0)
            
            if not buy_price or not sell_price or volume == 0:
                continue
            
            # Get more accurate buy order price from listings
            listings = self.steam.get_item_listings(item_name)
            if listings:
                buy_price = self._extract_highest_buy_order(listings)
                sell_price = self._extract_lowest_sell_listing(listings)
            
            analysis = analyzer.analyze_item(item_name, buy_price, sell_price, volume, sell_price)
            
            if analysis["profitable"]:
                # Check if we already have this item in inventory (don't buy more)
                if inventory.can_sell_item(item_name):
                    results.append({
                        "market_hash_name": item_name,
                        **analysis
                    })
                    print(f"  {Fore.GREEN}✓ {item_name}: ${buy_price:.2f}→${sell_price:.2f} ({analysis['profit_margin']:.1f}%){Style.RESET_ALL}")
                else:
                    print(f"  {Fore.YELLOW}⊘ {item_name}: Already in inventory (protected){Style.RESET_ALL}")
            else:
                print(f"  {Fore.RED}✗ {item_name}: {analysis['reason']}{Style.RESET_ALL}")
            
            # Update DB
            database.upsert_item({
                "market_hash_name": item_name,
                "app_id": 730,
                "buy_order_price": buy_price,
                "sell_listing_price": sell_price,
                "volume_24h": volume,
                "avg_price_24h": sell_price,
                "profitable": analysis["profitable"],
                "profit_margin": analysis["profit_margin"]
            })
        
        return sorted(results, key=lambda x: x["profit_margin"], reverse=True)
    
    def _get_target_items(self) -> List[str]:
        """Get list of items to monitor. Extend this list as needed."""
        # High-volume CS2 cases and common items
        return [
            "Revolution Case",
            "Recoil Case",
            "Dreams & Nightmares Case",
            "Snakebite Case",
            "Fracture Case",
            "Prisma 2 Case",
            "Chroma 3 Case",
            "Operation Riptide Case",
            "Operation Broken Fang Case",
            "Clutch Case",
            "Glove Case",
            "Gamma 2 Case",
            "Horizon Case",
            "Danger Zone Case",
            "CS20 Case",
            "Shattered Web Case",
            "Chroma 2 Case",
            "Falchion Case",
            "Shadow Case",
            "Operation Phoenix Weapon Case",
            "Sticker | Copenhagen Flames (Holo) | 2024",
            "Sticker | Team Spirit (Holo) | 2024",
            "Sticker | FaZe Clan (Holo) | 2024",
            "Sticker | Natus Vincere (Holo) | 2024",
            "Sticker | G2 Esports (Holo) | 2024",
        ]
    
    def _parse_price(self, price_str: Optional[str]) -> Optional[float]:
        if not price_str:
            return None
        try:
            return float(price_str.replace("$", "").replace(",", "").replace("USD", "").strip())
        except:
            return None
    
    def _extract_highest_buy_order(self, listings_data: Dict) -> float:
        # Parse buy orders from listings HTML - simplified
        return 0.03  # Placeholder - would parse actual buy orders
    
    def _extract_lowest_sell_listing(self, listings_data: Dict) -> float:
        # Parse sell listings from HTML - simplified
        return 0.05  # Placeholder
    
    def _process_flip(self, item: Dict):
        name = item["market_hash_name"]
        buy_price = item["buy_price"]
        sell_price = item["sell_price"]
        margin = item["profit_margin"]
        profit = item["net_profit_per_unit"]
        
        print(f"{Fore.CYAN}[Flipper] Processing: {name}{Style.RESET_ALL}")
        
        # Notify
        self.notifier.notify_flip_found(name, buy_price, sell_price, margin, profit, item["volume_24h"])
        
        # Place buy order (slightly above highest buy order)
        buy_price_cents = int(round(buy_price * 100))
        order_id = self.steam.create_buy_order(name, buy_price_cents, 1)
        
        if order_id:
            database.record_transaction({
                "market_hash_name": name,
                "action": "buy",
                "price": buy_price,
                "quantity": 1,
                "buy_order_id": order_id,
                "status": "pending"
            })
            self.notifier.notify_buy_placed(name, buy_price, 1, order_id)
            print(f"{Fore.GREEN}[Flipper] Buy order placed: {order_id}{Style.RESET_ALL}")
        else:
            print(f"{Fore.RED}[Flipper] Failed to place buy order{Style.RESET_ALL}")
            self.notifier.notify_error(f"buy_order_{name}", "Failed to place buy order")
    
    def _manage_existing_listings(self):
        """Check and manage active buy orders and sell listings."""
        pending = database.get_pending_transactions()
        
        for tx in pending:
            if not self.running:
                break
            
            if tx["action"] == "buy" and tx["buy_order_id"]:
                # Check if filled - would need to poll mybuyorders
                # For now, skip - implement polling in production
                pass
            
            elif tx["action"] == "sell" and tx["listing_id"]:
                # Check if sold - would need to poll mylistings
                pass

def main():
    flipper = SteamFlipper()
    flipper.start()

if __name__ == "__main__":
    main()