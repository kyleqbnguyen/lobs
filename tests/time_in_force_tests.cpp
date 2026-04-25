#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest,
       addOrder_shouldCancelIocRemainder_whenOrderPartiallyMatches) {
  addLimit(100, Side::Ask, 10000, 10);

  const auto trades = addLimit(200, Side::Bid, 10000, 15, TimeInForce::IOC);

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldCancelIocWithoutTrade_whenOrderDoesNotCross) {
  addLimit(100, Side::Ask, 10100, 10);

  const auto trades = addLimit(200, Side::Bid, 10000, 5, TimeInForce::IOC);

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 10);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldExecuteFokCompletely_whenOrderIsFullyFillable) {
  addLimit(100, Side::Ask, 10000, 5);
  addLimit(101, Side::Ask, 10100, 5);

  const auto trades = addLimit(200, Side::Bid, 10100, 10, TimeInForce::FOK);

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].quantity + trades[1].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldExecuteFokAgainstSingleLevel_whenExactLiquidityExists) {
  addLimit(100, Side::Ask, 10000, 10);

  const auto trades = addLimit(200, Side::Bid, 10000, 10, TimeInForce::FOK);

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldCancelFokCompletely_whenLiquidityIsInsufficient) {
  addLimit(100, Side::Ask, 10000, 5);

  const auto trades = addLimit(200, Side::Bid, 10000, 10, TimeInForce::FOK);

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest, addOrder_shouldCancelFokWithoutTrading_whenBookIsEmpty) {
  const auto trades = addLimit(200, Side::Bid, 10000, 10, TimeInForce::FOK);

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldCancelFokWithoutTrading_whenLiquidityExistsOnlyBeyondLimitPrice) {
  addLimit(100, Side::Ask, 10100, 10);

  const auto trades = addLimit(200, Side::Bid, 10000, 10, TimeInForce::FOK);

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 10);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldLeaveBookUnchanged_whenFokCannotFillAcrossMultipleLevels) {
  addLimit(100, Side::Ask, 10000, 5);
  addLimit(101, Side::Ask, 10100, 3);

  const auto trades = addLimit(200, Side::Bid, 10100, 10, TimeInForce::FOK);

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.depth(Side::Ask), 2);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 5);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 3);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldSweepAvailableLiquidityAndCancelRemainder_whenIocCrossesMultipleLevels) {
  addLimit(100, Side::Ask, 10000, 5);
  addLimit(101, Side::Ask, 10100, 4);

  const auto trades = addLimit(200, Side::Bid, 10100, 12, TimeInForce::IOC);

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].quantity, 4);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}
