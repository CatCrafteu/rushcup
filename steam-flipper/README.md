# Steam Market Flipper Bot

Conservative Steam market reseller for CS2 cases/skins/stickers. Runs locally on your main account with Telegram notifications.

## Features
- **Profit calculation** with 10% Steam tax + 5% buy order fee
- **Inventory protection** - never sells items you're using
- **Conservative rate limiting** - 8s between requests, safe for main account
- **Telegram notifications** - buy orders, fills, sales, daily summary
- **SQLite tracking** - full transaction history
- **Daily spend limits** - configurable budget protection

## Setup

### 1. Install dependencies
```bash
pip install -r requirements.txt
```

### 2. Get Steam credentials
You need:
- **Username/password** - your Steam login
- **Shared secret** - for 2FA code generation
- **Identity secret** - for trade confirmations
- **API key** - from https://steamcommunity.com/dev/apikey

**To get shared_secret + identity_secret:**
1. Use [Steam Desktop Authenticator](https://github.com/Jessecar96/SteamDesktopAuthenticator)
2. Add your account
3. Export `maFile` - it contains both secrets

### 3. Get Telegram Chat ID
1. Message your bot: `@your_bot_username`
2. Send `/start`
3. Visit: `https://api.telegram.org/bot<YOUR_BOT_TOKEN>/getUpdates`
4. Find `"chat":{"id":XXXXXX}` - that's your chat_id

### 4. Configure `config.json`
```json
{
  "steam": {
    "username": "your_steam_username",
    "password": "your_steam_password",
    "shared_secret": "base64_shared_secret_from_mafile",
    "identity_secret": "base64_identity_secret_from_mafile",
    "api_key": "YOUR_STEAM_API_KEY"
  },
  "telegram": {
    "bot_token": "8992360297:AAHGLE3ElRHd2o5aSPzdkJDLm4V7D9VRkIc",
    "chat_id": "YOUR_TELEGRAM_CHAT_ID"
  },
  "settings": {
    "min_profit_percent": 5.0,
    "max_buy_price": 50.0,
    "min_buy_price": 0.03,
    "max_daily_spend": 100.0,
    "max_concurrent_listings": 50,
    "scan_interval_seconds": 300,
    "request_delay_seconds": 8,
    "buy_order_duration_days": 1,
    "sell_price_undercut_percent": 1.0
  }
}
```

### 5. Protect your used items
Edit `config.json` and add asset IDs of items you're currently using:
```json
"manual_used_asset_ids": ["1234567890", "0987654321"]
```
Or run once, check the database, then mark items as used.

### 6. Run
```bash
python flipper.py
```

## How it works

1. **Scans** target items every 5 minutes (configurable)
2. **Checks** buy orders vs sell listings for spread
3. **Calculates** profit after 10% Steam tax + 5% buy fee
4. **Places buy orders** only if margin ≥ 5% (configurable)
5. **When filled**, automatically lists for sale at competitive price
6. **Notifies** via Telegram at every step

## Safety features

- **Never sells protected items** - tracks inventory, excludes marked assets
- **Daily spend cap** - stops buying at limit
- **Max concurrent listings** - prevents overextension
- **Conservative delays** - 8s between requests (Steam allows 20/min)
- **Main account friendly** - no aggressive patterns

## Adding more items to monitor

Edit `flipper.py` → `_get_target_items()` method. Add any market_hash_name from Steam market URLs.

Example: `https://steamcommunity.com/market/listings/730/Revolution%20Case`
→ market_hash_name = `"Revolution Case"`

## Database

`flipper.db` contains:
- `items` - price history & profitability
- `transactions` - every buy/sell with fees & profit
- `inventory_snapshot` - your items with protection flags
- `bot_state` - runtime settings

## Logs

Watch console for real-time status. Telegram gets key events.

## Stopping

`Ctrl+C` - gracefully shuts down, cancels no orders (they expire naturally).

## Disclaimer

Steam ToS prohibits automated market interaction. Use at your own risk. This bot uses conservative settings but bans are possible. Test with small amounts first.