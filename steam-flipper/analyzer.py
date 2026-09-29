from typing import Optional, Tuple
import config
import database

STEAM_TAX_RATE = 0.10  # 10% Steam market fee
BUY_ORDER_FEE_RATE = 0.05  # 5% buy order fee (goes to Steam)

def calculate_profit(buy_price: float, sell_price: float, quantity: int = 1) -> Tuple[float, float, float]:
    """
    Calculate net profit after all fees.
    Returns: (gross_profit, total_fees, net_profit)
    """
    gross_revenue = sell_price * quantity
    steam_tax = gross_revenue * STEAM_TAX_RATE
    buy_fee = buy_price * quantity * BUY_ORDER_FEE_RATE
    total_cost = (buy_price * quantity) + buy_fee
    total_fees = steam_tax + buy_fee
    net_profit = gross_revenue - total_cost - steam_tax
    return gross_revenue - total_cost, total_fees, net_profit

def calculate_profit_margin(buy_price: float, sell_price: float) -> float:
    """Calculate profit margin percentage after all fees."""
    if buy_price <= 0:
        return 0.0
    _, _, net_profit = calculate_profit(buy_price, sell_price)
    return (net_profit / buy_price) * 100

def is_profitable(buy_price: float, sell_price: float, min_margin: float = None) -> bool:
    """Check if a flip is profitable after fees."""
    if min_margin is None:
        min_margin = config.config["settings"]["min_profit_percent"]
    margin = calculate_profit_margin(buy_price, sell_price)
    return margin >= min_margin

def calculate_optimal_buy_price(sell_price: float, target_margin: float = None) -> float:
    """Calculate max buy price to achieve target margin after fees."""
    if target_margin is None:
        target_margin = config.config["settings"]["min_profit_percent"]
    # sell_price * (1 - tax) = revenue_after_tax
    # revenue_after_tax - buy_price * (1 + buy_fee) = profit
    # profit / buy_price = target_margin / 100
    # Solving for buy_price:
    revenue_after_tax = sell_price * (1 - STEAM_TAX_RATE)
    buy_price = revenue_after_tax / (1 + BUY_ORDER_FEE_RATE + target_margin / 100)
    return round(buy_price, 2)

def calculate_optimal_sell_price(buy_price: float, target_margin: float = None) -> float:
    """Calculate min sell price to achieve target margin after fees."""
    if target_margin is None:
        target_margin = config.config["settings"]["min_profit_percent"]
    # buy_price * (1 + buy_fee) = total_cost
    # sell_price * (1 - tax) - total_cost = profit
    # profit / buy_price = target_margin / 100
    total_cost = buy_price * (1 + BUY_ORDER_FEE_RATE)
    required_revenue = total_cost * (1 + target_margin / 100)
    sell_price = required_revenue / (1 - STEAM_TAX_RATE)
    return round(sell_price, 2)

def analyze_item(market_hash_name: str, buy_order_price: float, sell_listing_price: float, 
                 volume_24h: int, avg_price_24h: float) -> dict:
    """Full analysis of an item for flipping."""
    min_buy = config.config["settings"]["min_buy_price"]
    max_buy = config.config["settings"]["max_buy_price"]
    
    if buy_order_price < min_buy or buy_order_price > max_buy:
        return {"profitable": False, "reason": "buy_price_out_of_range"}
    
    if sell_listing_price <= buy_order_price:
        return {"profitable": False, "reason": "no_spread"}
    
    if volume_24h < config.config["filters"].get("min_volume_24h", 10):
        return {"profitable": False, "reason": "low_volume"}
    
    margin = calculate_profit_margin(buy_order_price, sell_listing_price)
    _, _, net_profit = calculate_profit(buy_order_price, sell_listing_price)
    
    profitable = margin >= config.config["settings"]["min_profit_percent"]
    
    return {
        "profitable": profitable,
        "buy_price": buy_order_price,
        "sell_price": sell_listing_price,
        "volume_24h": volume_24h,
        "avg_price_24h": avg_price_24h,
        "profit_margin": round(margin, 2),
        "net_profit_per_unit": round(net_profit, 2),
        "steam_tax_per_unit": round(sell_listing_price * STEAM_TAX_RATE, 2),
        "buy_fee_per_unit": round(buy_order_price * BUY_ORDER_FEE_RATE, 2),
        "reason": "profitable" if profitable else "insufficient_margin"
    }

def get_price_tier_recommendation(analysis: dict) -> dict:
    """Get recommended buy/sell prices for different risk tiers."""
    buy = analysis["buy_price"]
    sell = analysis["sell_price"]
    min_margin = config.config["settings"]["min_profit_percent"]
    
    return {
        "conservative": {
            "buy": calculate_optimal_buy_price(sell, min_margin + 2),
            "sell": sell,
            "margin": min_margin + 2
        },
        "balanced": {
            "buy": calculate_optimal_buy_price(sell, min_margin),
            "sell": sell,
            "margin": min_margin
        },
        "aggressive": {
            "buy": buy,
            "sell": calculate_optimal_sell_price(buy, min_margin),
            "margin": min_margin
        }
    }