#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest,
       addOrder_shouldFillAtPassivePrice_whenMarketBidCrossesBook) {
  addLimit(100, Side::Ask, 10000, 10);

  const auto trades = addMarket(200, Side::Bid, 10);

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldSweepAllBidLevels_whenMarketAskEntersBook) {
  addLimit(100, Side::Bid, 10100, 5);
  addLimit(101, Side::Bid, 10000, 4);

  const auto trades = addMarket(200, Side::Ask, 9);

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].price, 10100);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].price, 10000);
  EXPECT_EQ(trades[1].quantity, 4);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldNotRestUnfilledQuantity_whenGtcMarketBidExhaustsLiquidity) {
  addLimit(100, Side::Ask, 10000, 5);

  const auto trades = addMarket(200, Side::Bid, 10, TimeInForce::GTC);

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldNotRestUnfilledQuantity_whenGtcMarketBidEntersEmptyBook) {
  const auto trades = addMarket(200, Side::Bid, 10, TimeInForce::GTC);

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldExecuteMarketFokBid_whenSufficientLiquidityExists) {
  addLimit(100, Side::Ask, 10000, 5);
  addLimit(101, Side::Ask, 10100, 5);

  const auto trades = addMarket(200, Side::Bid, 10, TimeInForce::FOK);

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldCancelMarketFokBidWithoutTrading_whenLiquidityIsInsufficient) {
  addLimit(100, Side::Ask, 10000, 5);

  const auto trades = addMarket(200, Side::Bid, 10, TimeInForce::FOK);

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}
