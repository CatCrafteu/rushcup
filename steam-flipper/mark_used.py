#!/usr/bin/env python3
"""
Interactive tool to mark inventory items as 'used/protected' so the flipper won't sell them.
Run after first inventory sync to select your active loadout items.
"""

import sqlite3
from pathlib import Path

DB_PATH = Path(__file__).parent / "flipper.db"

def main():
    with sqlite3.connect(DB_PATH) as conn:
        conn.row_factory = sqlite3.Row
        
        # Get all unique marketable items in inventory
        rows = conn.execute("""
            SELECT DISTINCT market_hash_name, asset_id, class_id
            FROM inventory_snapshot
            WHERE marketable = 1 AND tradable = 1
            ORDER BY market_hash_name
        """).fetchall()
        
        if not rows:
            print("No inventory data found. Run flipper.py once first to sync inventory.")
            return
        
        print(f"\nFound {len(rows)} marketable items in your inventory.\n")
        print("Enter asset IDs to mark as PROTECTED (won't be sold), one per line.")
        print("Press Enter on empty line when done.\n")
        
        # Group by item name for easier viewing
        by_name = {}
        for row in rows:
            name = row["market_hash_name"]
            if name not in by_name:
                by_name[name] = []
            by_name[name].append(row["asset_id"])
        
        for name, asset_ids in by_name.items():
            print(f"  {name}")
            for aid in asset_ids:
                print(f"    Asset ID: {aid}")
        
        print("\n--- Enter asset IDs to protect ---")
        protected = []
        while True:
            user_input = input("Asset ID (or Enter to finish): ").strip()
            if not user_input:
                break
            protected.append(user_input)
        
        if not protected:
            print("No items marked.")
            return
        
        # Update database
        placeholders = ",".join("?" * len(protected))
        conn.execute(f"""
            UPDATE inventory_snapshot 
            SET is_used_by_owner = 1 
            WHERE asset_id IN ({placeholders})
        """, protected)
        
        print(f"\n{Fore.GREEN}Marked {len(protected)} items as protected.{Style.RESET_ALL}")
        
        # Show summary
        protected_rows = conn.execute(f"""
            SELECT market_hash_name, asset_id FROM inventory_snapshot
            WHERE asset_id IN ({placeholders}) AND is_used_by_owner = 1
        """, protected).fetchall()
        
        for row in protected_rows:
            print(f"  ✓ {row['market_hash_name']} ({row['asset_id']})")

if __name__ == "__main__":
    from colorama import Fore, Style, init
    init(autoreset=True)
    main()