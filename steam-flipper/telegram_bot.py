import asyncio
import json
from typing import Optional
from telegram import Bot
from telegram.error import TelegramError
import config
import database

class TelegramNotifier:
    def __init__(self):
        self.bot = Bot(token=config.config["telegram"]["bot_token"])
        self.chat_id = config.config["telegram"]["chat_id"]
        self.enabled = bool(self.chat_id)
    
    async def send(self, text: str, parse_mode: str = "HTML") -> bool:
        if not self.enabled:
            return False
        try:
            await self.bot.send_message(chat_id=self.chat_id, text=text, parse_mode=parse_mode)
            return True
        except TelegramError as e:
            print(f"[Telegram] Send failed: {e}")
            return False
    
    def send_sync(self, text: str, parse_mode: str = "HTML") -> bool:
        if not self.enabled:
            return False
        try:
            loop = asyncio.get_event_loop()
        except RuntimeError:
            loop = asyncio.new_event_loop()
            asyncio.set_event_loop(loop)
        return loop.run_until_complete(self.send(text, parse_mode))

    def notify_flip_found(self, item_name: str, buy_price: float, sell_price: float, 
                          margin: float, profit: float, volume: int) -> bool:
        text = (
            f"🔥 <b>Profitable Flip Found</b>\n\n"
            f"<b>Item:</b> {item_name}\n"
            f"<b>Buy:</b> ${buy_price:.2f} → <b>Sell:</b> ${sell_price:.2f}\n"
            f"<b>Margin:</b> {margin:.1f}% | <b>Net Profit:</b> ${profit:.2f}/unit\n"
            f"<b>24h Volume:</b> {volume}\n\n"
            f"<i>Placing buy order...</i>"
        )
        return self.send_sync(text)
    
    def notify_buy_placed(self, item_name: str, price: float, quantity: int, order_id: str) -> bool:
        text = (
            f"✅ <b>Buy Order Placed</b>\n\n"
            f"<b>Item:</b> {item_name}\n"
            f"<b>Price:</b> ${price:.2f} x {quantity}\n"
            f"<b>Order ID:</b> <code>{order_id}</code>"
        )
        return self.send_sync(text)
    
    def notify_buy_filled(self, item_name: str, price: float, quantity: int) -> bool:
        text = (
            f"📦 <b>Buy Order Filled</b>\n\n"
            f"<b>Item:</b> {item_name}\n"
            f"<b>Bought at:</b> ${price:.2f} x {quantity}\n"
            f"<i>Listing for sale...</i>"
        )
        return self.send_sync(text)
    
    def notify_listed(self, item_name: str, price: float, listing_id: str) -> bool:
        text = (
            f"📋 <b>Listed for Sale</b>\n\n"
            f"<b>Item:</b> {item_name}\n"
            f"<b>Price:</b> ${price:.2f}\n"
            f"<b>Listing ID:</b> <code>{listing_id}</code>"
        )
        return self.send_sync(text)
    
    def notify_sold(self, item_name: str, buy_price: float, sell_price: float, 
                    net_profit: float, steam_tax: float) -> bool:
        text = (
            f"💰 <b>Item Sold!</b>\n\n"
            f"<b>Item:</b> {item_name}\n"
            f"<b>Bought:</b> ${buy_price:.2f} | <b>Sold:</b> ${sell_price:.2f}\n"
            f"<b>Steam Tax:</b> ${steam_tax:.2f}\n"
            f"<b>Net Profit:</b> ${net_profit:.2f} ✅"
        )
        return self.send_sync(text)
    
    def notify_daily_summary(self) -> bool:
        today_spend = database.get_daily_spend()
        pending = database.get_pending_transactions()
        active_listings = database.get_active_listings_count()
        
        total_profit = 0
        with database.get_db() as conn:
            row = conn.execute("""
                SELECT COALESCE(SUM(profit), 0) FROM transactions 
                WHERE action = 'sell' AND status = 'sold' 
                AND completed_at >= date('now')
            """).fetchone()
            if row:
                total_profit = row[0]
        
        text = (
            f"📊 <b>Daily Summary</b>\n\n"
            f"<b>Spent Today:</b> ${today_spend:.2f}\n"
            f"<b>Profit Today:</b> ${total_profit:.2f}\n"
            f"<b>Active Listings:</b> {active_listings}\n"
            f"<b>Pending Orders:</b> {len(pending)}"
        )
        return self.send_sync(text)
    
    def notify_error(self, context: str, error: str) -> bool:
        text = f"⚠️ <b>Error</b> in {context}\n<code>{error}</code>"
        return self.send_sync(text)
    
    def notify_startup(self) -> bool:
        text = (
            f"🤖 <b>Flipper Started</b>\n\n"
            f"<b>Min Profit:</b> {config.config['settings']['min_profit_percent']}%\n"
            f"<b>Max Buy:</b> ${config.config['settings']['max_buy_price']:.2f}\n"
            f"<b>Max Daily Spend:</b> ${config.config['settings']['max_daily_spend']:.2f}\n"
            f"<b>Scan Interval:</b> {config.config['settings']['scan_interval_seconds']}s"
        )
        return self.send_sync(text)
    
    def notify_shutdown(self) -> bool:
        text = "🛑 <b>Flipper Stopped</b>"
        return self.send_sync(text)

notifier = TelegramNotifier()