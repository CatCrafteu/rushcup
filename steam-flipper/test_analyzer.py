#!/usr/bin/env python3
"""Test profit calculations with various scenarios."""

import analyzer

def test_cases():
    print("=== Profit Calculator Tests ===\n")
    
    # Test 1: Basic case flip
    print("Test 1: Case buy $0.50, sell $0.70")
    buy, sell = 0.50, 0.70
    gross, fees, net = analyzer.calculate_profit(buy, sell)
    margin = analyzer.calculate_profit_margin(buy, sell)
    print(f"  Gross: ${gross:.2f}, Fees: ${fees:.2f}, Net: ${net:.2f}, Margin: {margin:.1f}%")
    print(f"  Profitable (5% min): {analyzer.is_profitable(buy, sell)}")
    
    # Test 2: Sticker flip
    print("\nTest 2: Sticker buy $1.00, sell $1.50")
    buy, sell = 1.00, 1.50
    gross, fees, net = analyzer.calculate_profit(buy, sell)
    margin = analyzer.calculate_profit_margin(buy, sell)
    print(f"  Gross: ${gross:.2f}, Fees: ${fees:.2f}, Net: ${net:.2f}, Margin: {margin:.1f}%")
    
    # Test 3: Skin flip
    print("\nTest 3: Skin buy $10.00, sell $13.00")
    buy, sell = 10.00, 13.00
    gross, fees, net = analyzer.calculate_profit(buy, sell)
    margin = analyzer.calculate_profit_margin(buy, sell)
    print(f"  Gross: ${gross:.2f}, Fees: ${fees:.2f}, Net: ${net:.2f}, Margin: {margin:.1f}%")
    
    # Test 4: Marginal case
    print("\nTest 4: Marginal buy $0.50, sell $0.55")
    buy, sell = 0.50, 0.55
    gross, fees, net = analyzer.calculate_profit(buy, sell)
    margin = analyzer.calculate_profit_margin(buy, sell)
    print(f"  Gross: ${gross:.2f}, Fees: ${fees:.2f}, Net: ${net:.2f}, Margin: {margin:.1f}%")
    print(f"  Profitable (5% min): {analyzer.is_profitable(buy, sell)}")
    
    # Test 5: Optimal pricing
    print("\nTest 5: Optimal pricing for $1.00 sell target")
    sell = 1.00
    opt_buy = analyzer.calculate_optimal_buy_price(sell, 5.0)
    print(f"  Max buy for 5% margin: ${opt_buy:.2f}")
    margin_check = analyzer.calculate_profit_margin(opt_buy, sell)
    print(f"  Actual margin: {margin_check:.1f}%")
    
    # Test 6: Full analysis
    print("\nTest 6: Full item analysis")
    analysis = analyzer.analyze_item("Test Case", 0.50, 0.75, 100, 0.70)
    print(f"  {analysis}")
    
    recs = analyzer.get_price_tier_recommendation(analysis)
    for tier, prices in recs.items():
        print(f"  {tier}: buy ${prices['buy']:.2f}, sell ${prices['sell']:.2f}, target {prices['margin']}%")

if __name__ == "__main__":
    test_cases()