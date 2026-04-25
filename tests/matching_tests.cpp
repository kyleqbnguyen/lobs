#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest, addOrder_shouldConsumeRestingAsk_whenBidCrossesBook) {
  addLimit(100, Side::Ask, 10000, 10);

  const auto trades = addLimit(200, Side::Bid, 10000, 10);

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 10);

  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.depth(Side::Bid), 0);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldRestBidRemainder_whenBidPartiallyFillsAsk) {
  addLimit(100, Side::Ask, 10000, 10);

  const auto trades = addLimit(200, Side::Bid, 10000, 15);

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest, addOrder_shouldPromoteNextBid_whenBestBidIsFullyFilled) {
  addLimit(100, Side::Bid, 10000, 10);
  addLimit(101, Side::Bid, 9900, 7);

  const auto trades = addLimit(200, Side::Ask, 10000, 10);

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(orderBook.bestBid(), 9900);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 9900), 7);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldMatchHighestBidFirst_whenAskSweepsMultipleBidLevels) {
  addLimit(100, Side::Bid, 10100, 5);
  addLimit(101, Side::Bid, 10000, 7);

  const auto trades = addLimit(200, Side::Ask, 10000, 10);

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10100);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].price, 10000);
  EXPECT_EQ(trades[1].quantity, 5);

  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 2);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldUseFifo_whenMatchingRestingAsksAtSamePrice) {
  addLimit(100, Side::Ask, 10000, 5);
  addLimit(101, Side::Ask, 10000, 7);

  const auto trades = addLimit(200, Side::Bid, 10000, 6);

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].quantity, 1);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 6);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldRestAggressorRemainder_whenSweepExhaustsAvailableLiquidity) {
  addLimit(100, Side::Ask, 10000, 5);
  addLimit(101, Side::Ask, 10100, 7);

  const auto trades = addLimit(200, Side::Bid, 10100, 15);

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].price, 10100);
  EXPECT_EQ(trades[1].quantity, 7);

  EXPECT_EQ(orderBook.bestBid(), 10100);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10100), 3);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 0);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldTradeAtPassivePrice_whenAggressorImprovesOnRestingAsk) {
  addLimit(100, Side::Ask, 10000, 5);

  const auto trades = addLimit(200, Side::Bid, 10500, 5);

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldLeaveSameSideLiquidityUntouched_whenMatchingOppositeSide) {
  addLimit(10, Side::Bid, 9900, 4);
  addLimit(100, Side::Ask, 10000, 5);

  const auto trades = addLimit(200, Side::Bid, 10000, 5);

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(orderBook.bestBid(), 9900);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 9900), 4);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldStopAtLimitPrice_whenLimitBidCannotSweepHigherAskLevels) {
  addLimit(100, Side::Ask, 10000, 5);
  addLimit(101, Side::Ask, 10100, 7);

  const auto trades = addLimit(200, Side::Bid, 10000, 10);

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 7);
  EXPECT_EQ(orderBook.orderCount(), 2);
}
