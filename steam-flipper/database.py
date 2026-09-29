import sqlite3
import json
from datetime import datetime
from pathlib import Path
from typing import Optional, List, Dict, Any
from dataclasses import dataclass
from contextlib import contextmanager

DB_PATH = Path(__file__).parent / "flipper.db"

SCHEMA = """
PRAGMA journal_mode=WAL;
PRAGMA foreign_keys=ON;

CREATE TABLE IF NOT EXISTS items (
    market_hash_name TEXT PRIMARY KEY,
    app_id INTEGER NOT NULL,
    type TEXT,
    rarity TEXT,
    last_price_check REAL,
    avg_price_24h REAL,
    volume_24h INTEGER,
    buy_order_price REAL,
    sell_listing_price REAL,
    profitable BOOLEAN DEFAULT 0,
    profit_margin REAL,
    updated_at REAL
);

CREATE TABLE IF NOT EXISTS transactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    market_hash_name TEXT NOT NULL,
    action TEXT NOT NULL,
    price REAL NOT NULL,
    quantity INTEGER DEFAULT 1,
    fee REAL,
    profit REAL,
    steam_tax REAL,
    listing_id TEXT,
    buy_order_id TEXT,
    status TEXT DEFAULT 'pending',
    created_at REAL NOT NULL,
    completed_at REAL,
    FOREIGN KEY (market_hash_name) REFERENCES items(market_hash_name)
);

CREATE TABLE IF NOT EXISTS inventory_snapshot (
    asset_id TEXT PRIMARY KEY,
    market_hash_name TEXT NOT NULL,
    class_id TEXT,
    instance_id TEXT,
    amount INTEGER DEFAULT 1,
    tradable BOOLEAN,
    marketable BOOLEAN,
    app_id INTEGER,
    context_id INTEGER,
    is_used_by_owner BOOLEAN DEFAULT 0,
    snapshot_at REAL NOT NULL
);

CREATE TABLE IF NOT EXISTS bot_state (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL,
    updated_at REAL NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_transactions_name ON transactions(market_hash_name);
CREATE INDEX IF NOT EXISTS idx_transactions_status ON transactions(status);
CREATE INDEX IF NOT EXISTS idx_items_profitable ON items(profitable);
CREATE INDEX IF NOT EXISTS idx_inventory_hash ON inventory_snapshot(market_hash_name);
"""

@contextmanager
def get_db():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    try:
        yield conn
        conn.commit()
    except Exception:
        conn.rollback()
        raise
    finally:
        conn.close()

def init_db():
    with get_db() as conn:
        conn.executescript(SCHEMA)

def set_state(key: str, value: Any):
    with get_db() as conn:
        conn.execute(
            "INSERT OR REPLACE INTO bot_state (key, value, updated_at) VALUES (?, ?, ?)",
            (key, json.dumps(value), datetime.now().timestamp())
        )

def get_state(key: str, default=None):
    with get_db() as conn:
        row = conn.execute("SELECT value FROM bot_state WHERE key = ?", (key,)).fetchone()
        return json.loads(row[0]) if row else default

def upsert_item(item: Dict):
    with get_db() as conn:
        conn.execute("""
            INSERT OR REPLACE INTO items 
            (market_hash_name, app_id, type, rarity, last_price_check, avg_price_24h, volume_24h,
             buy_order_price, sell_listing_price, profitable, profit_margin, updated_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            item["market_hash_name"], item["app_id"], item.get("type"),
            item.get("rarity"), item.get("last_price_check"), item.get("avg_price_24h"),
            item.get("volume_24h"), item.get("buy_order_price"), item.get("sell_listing_price"),
            item.get("profitable", False), item.get("profit_margin", 0), datetime.now().timestamp()
        ))

def get_profitable_items(limit: int = 100) -> List[sqlite3.Row]:
    with get_db() as conn:
        return conn.execute("""
            SELECT * FROM items WHERE profitable = 1 AND volume_24h >= ?
            ORDER BY profit_margin DESC LIMIT ?
        """, (get_state("min_volume_24h", 10), limit)).fetchall()

def get_item(market_hash_name: str) -> Optional[sqlite3.Row]:
    with get_db() as conn:
        return conn.execute("SELECT * FROM items WHERE market_hash_name = ?", (market_hash_name,)).fetchone()

def record_transaction(tx: Dict):
    with get_db() as conn:
        conn.execute("""
            INSERT INTO transactions 
            (market_hash_name, action, price, quantity, fee, profit, steam_tax, listing_id, buy_order_id, status, created_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            tx["market_hash_name"], tx["action"], tx["price"], tx.get("quantity", 1),
            tx.get("fee", 0), tx.get("profit", 0), tx.get("steam_tax", 0),
            tx.get("listing_id"), tx.get("buy_order_id"), tx.get("status", "pending"),
            datetime.now().timestamp()
        ))

def update_transaction(listing_id: str, status: str, profit: float = 0):
    with get_db() as conn:
        conn.execute(
            "UPDATE transactions SET status = ?, profit = ?, completed_at = ? WHERE listing_id = ? OR buy_order_id = ?",
            (status, profit, datetime.now().timestamp(), listing_id, listing_id)
        )

def get_pending_transactions() -> List[sqlite3.Row]:
    with get_db() as conn:
        return conn.execute("SELECT * FROM transactions WHERE status = 'pending'").fetchall()

def save_inventory_snapshot(items: List[Dict], used_asset_ids: set):
    with get_db() as conn:
        conn.execute("DELETE FROM inventory_snapshot")
        now = datetime.now().timestamp()
        for item in items:
            asset_id = item.get("asset_id") or f"{item['class_id']}_{item['instance_id']}"
            is_used = asset_id in used_asset_ids
            conn.execute("""
                INSERT INTO inventory_snapshot 
                (asset_id, market_hash_name, class_id, instance_id, amount, tradable, marketable, 
                 app_id, context_id, is_used_by_owner, snapshot_at)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                asset_id, item["market_hash_name"], item.get("class_id"), item.get("instance_id"),
                item.get("amount", 1), item.get("tradable", False), item.get("marketable", False),
                item.get("app_id"), item.get("context_id"), is_used, now
            ))

def is_item_used_by_owner(market_hash_name: str, exclude_asset_ids: set = None) -> bool:
    with get_db() as conn:
        if exclude_asset_ids:
            placeholders = ",".join("?" * len(exclude_asset_ids))
            query = f"SELECT 1 FROM inventory_snapshot WHERE market_hash_name = ? AND is_used_by_owner = 1 AND asset_id NOT IN ({placeholders})"
            params = [market_hash_name] + list(exclude_asset_ids)
        else:
            query = "SELECT 1 FROM inventory_snapshot WHERE market_hash_name = ? AND is_used_by_owner = 1"
            params = [market_hash_name]
        return conn.execute(query, params).fetchone() is not None

def get_used_asset_ids() -> set:
    with get_db() as conn:
        rows = conn.execute("SELECT asset_id FROM inventory_snapshot WHERE is_used_by_owner = 1").fetchall()
        return {r[0] for r in rows}

def get_daily_spend() -> float:
    today_start = datetime.now().replace(hour=0, minute=0, second=0, microsecond=0).timestamp()
    with get_db() as conn:
        row = conn.execute("""
            SELECT COALESCE(SUM(price * quantity), 0) FROM transactions 
            WHERE action = 'buy' AND created_at >= ?
        """, (today_start,)).fetchone()
        return row[0] if row else 0.0

def get_active_listings_count() -> int:
    with get_db() as conn:
        row = conn.execute("SELECT COUNT(*) FROM transactions WHERE action = 'sell' AND status IN ('listed', 'pending')").fetchone()
        return row[0] if row else 0

init_db()