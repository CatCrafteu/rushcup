import time
import json
import requests
from typing import Dict, List, Optional, Any
from dataclasses import dataclass
from steam import SteamClient
from steam.enums import EResult
from steam.webauth import WebAuth
from steam.webapi import WebAPI
from colorama import Fore, Style
import config

@dataclass
class MarketItem:
    market_hash_name: str
    app_id: int
    buy_order_price: float
    sell_listing_price: float
    volume_24h: int
    avg_price_24h: float

@dataclass
class InventoryItem:
    asset_id: str
    market_hash_name: str
    class_id: str
    instance_id: str
    amount: int
    tradable: bool
    marketable: bool
    app_id: int
    context_id: int

class SteamMarketClient:
    BASE_URL = "https://steamcommunity.com/market"
    API_URL = "https://api.steampowered.com"
    
    def __init__(self):
        self.client = SteamClient()
        self.webauth = None
        self.session = requests.Session()
        self.session.headers.update({
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"
        })
        self.logged_in = False
        self.steam_id = None
        self._request_count = 0
        self._last_request = 0
        
    def login(self) -> bool:
        print(f"{Fore.CYAN}[Steam] Logging in...{Style.RESET_ALL}")
        
        result = self.client.login(
            config.config["steam"]["username"],
            config.config["steam"]["password"],
            two_factor_code=self._get_2fa_code()
        )
        
        if result != EResult.OK:
            print(f"{Fore.RED}[Steam] Login failed: {result}{Style.RESET_ALL}")
            return False
            
        self.steam_id = str(self.client.steam_id)
        self.webauth = WebAuth(self.client)
        self._setup_session_cookies()
        self.logged_in = True
        print(f"{Fore.GREEN}[Steam] Logged in as {self.steam_id}{Style.RESET_ALL}")
        return True
    
    def _get_2fa_code(self) -> str:
        from steam.guard import generate_twofactor_code
        import base64
        shared_secret = base64.b64decode(config.config["steam"]["shared_secret"])
        return generate_twofactor_code(shared_secret)
    
    def _setup_session_cookies(self):
        cookies = self.webauth.get_session_cookies()
        for cookie in cookies:
            self.session.cookies.set(cookie.name, cookie.value, domain=cookie.domain)
        self.session.headers.update({"Referer": "https://steamcommunity.com/"})
    
    def _rate_limit(self):
        delay = config.config["settings"]["request_delay_seconds"]
        elapsed = time.time() - self._last_request
        if elapsed < delay:
            time.sleep(delay - elapsed)
        self._last_request = time.time()
        self._request_count += 1
    
    def _make_request(self, method: str, url: str, **kwargs) -> Optional[Dict]:
        self._rate_limit()
        try:
            resp = self.session.request(method, url, timeout=30, **kwargs)
            if resp.status_code == 429:
                print(f"{Fore.YELLOW}[Steam] Rate limited, backing off...{Style.RESET_ALL}")
                time.sleep(60)
                return self._make_request(method, url, **kwargs)
            resp.raise_for_status()
            return resp.json()
        except Exception as e:
            print(f"{Fore.RED}[Steam] Request failed: {e}{Style.RESET_ALL}")
            return None
    
    def get_price_overview(self, market_hash_name: str, app_id: int = 730) -> Optional[Dict]:
        url = f"{self.BASE_URL}/priceoverview/"
        params = {
            "country": "US",
            "currency": 1,
            "appid": app_id,
            "market_hash_name": market_hash_name
        }
        return self._make_request("GET", url, params=params)
    
    def get_item_listings(self, market_hash_name: str, app_id: int = 730) -> Optional[Dict]:
        url = f"{self.BASE_URL}/listings/{app_id}/{requests.utils.quote(market_hash_name)}/render/"
        params = {"query": "", "start": 0, "count": 10, "country": "US", "language": "en", "currency": 1}
        return self._make_request("GET", url, params=params)
    
    def get_my_listings(self) -> Optional[Dict]:
        url = f"{self.BASE_URL}/mylistings/render/"
        params = {"query": "", "start": 0, "count": 100, "country": "US", "language": "en", "currency": 1}
        return self._make_request("GET", url, params=params)
    
    def get_buy_orders(self) -> Optional[Dict]:
        url = f"{self.BASE_URL}/mybuyorders/render/"
        params = {"query": "", "start": 0, "count": 100, "country": "US", "language": "en", "currency": 1}
        return self._make_request("GET", url, params=params)
    
    def get_inventory(self, app_id: int = 730, context_id: int = 2) -> List[InventoryItem]:
        url = f"{self.API_URL}/IEconItems_{app_id}/GetPlayerItems/v1/"
        params = {"key": config.config["steam"]["api_key"], "steamid": self.steam_id}
        data = self._make_request("GET", url, params=params)
        
        if not data or "result" not in data or "items" not in data["result"]:
            return []
        
        items = []
        for item in data["result"]["items"]:
            items.append(InventoryItem(
                asset_id=str(item.get("id", "")),
                market_hash_name=item.get("market_hash_name", ""),
                class_id=str(item.get("defindex", "")),
                instance_id="0",
                amount=item.get("quantity", 1),
                tradable=item.get("flags", 0) & 1 == 1,
                marketable=item.get("flags", 0) & 2 == 2,
                app_id=app_id,
                context_id=context_id
            ))
        return items
    
    def create_buy_order(self, market_hash_name: str, price_cents: int, quantity: int = 1, app_id: int = 730) -> Optional[str]:
        url = f"{self.BASE_URL}/createbuyorder/"
        data = {
            "sessionid": self.session.cookies.get("sessionid", ""),
            "appid": app_id,
            "market_hash_name": market_hash_name,
            "price_total": price_cents * quantity,
            "quantity": quantity,
            "currency": 1
        }
        result = self._make_request("POST", url, data=data)
        if result and result.get("success"):
            return result.get("buy_order_id")
        return None
    
    def create_sell_listing(self, asset_id: str, price_cents: int, app_id: int = 730, context_id: int = 2) -> Optional[str]:
        url = f"{self.BASE_URL}/sellitem/"
        data = {
            "sessionid": self.session.cookies.get("sessionid", ""),
            "appid": app_id,
            "contextid": context_id,
            "assetid": asset_id,
            "amount": 1,
            "price": price_cents
        }
        result = self._make_request("POST", url, data=data)
        if result and result.get("success"):
            return result.get("listing_id")
        return None
    
    def cancel_buy_order(self, buy_order_id: str) -> bool:
        url = f"{self.BASE_URL}/cancelbuyorder/"
        data = {"sessionid": self.session.cookies.get("sessionid", ""), "buy_order_id": buy_order_id}
        result = self._make_request("POST", url, data=data)
        return result and result.get("success", False)
    
    def cancel_sell_listing(self, listing_id: str) -> bool:
        url = f"{self.BASE_URL}/removelisting/"
        data = {"sessionid": self.session.cookies.get("sessionid", ""), "listing_id": listing_id}
        result = self._make_request("POST", url, data=data)
        return result and result.get("success", False)
    
    def get_wallet_balance(self) -> float:
        url = f"{self.BASE_URL}/wallet_info/"
        data = self._make_request("GET", url)
        if data and "wallet_balance" in data:
            return data["wallet_balance"] / 100
        return 0.0

steam_client = SteamMarketClient()