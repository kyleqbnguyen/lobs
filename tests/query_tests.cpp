#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest, queries_shouldReportNoLiquidity_whenBookIsEmpty) {
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);

  EXPECT_EQ(orderBook.depth(Side::Bid), 0);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);

  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);

  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       queries_shouldReportLiquidityOnlyOnBidSide_whenBookHasSingleBid) {
  addLimit(100, Side::Bid, 10000, 10);

  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);

  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);

  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 10);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);

  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       bestBid_shouldReturnHighestRestingBid_whenBookHasMultipleBids) {
  addLimit(100, Side::Bid, 5000, 10);
  addLimit(101, Side::Bid, 10000, 10);

  EXPECT_EQ(orderBook.bestBid(), 10000);
}

TEST_F(OrderBookTest,
       bestAsk_shouldReturnLowestRestingAsk_whenBookHasMultipleAsks) {
  addLimit(100, Side::Ask, 10000, 10);
  addLimit(101, Side::Ask, 5000, 10);

  EXPECT_EQ(orderBook.bestAsk(), 5000);
}

TEST_F(OrderBookTest, depth_shouldNotIncrease_whenBidAddedAtSamePriceLevel) {
  addLimit(100, Side::Bid, 10000, 10);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);

  addLimit(101, Side::Bid, 10000, 5);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
}

TEST_F(OrderBookTest, depth_shouldIncrease_whenBidAddedOnNewLevel) {
  EXPECT_EQ(orderBook.depth(Side::Bid), 0);

  addLimit(100, Side::Bid, 10000, 10);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);

  addLimit(101, Side::Bid, 5000, 5);
  EXPECT_EQ(orderBook.depth(Side::Bid), 2);
}

TEST_F(OrderBookTest,
       depth_shouldRemainIndependent_whenBothSidesHaveLiquidity) {
  addLimit(100, Side::Bid, 10000, 10);
  addLimit(101, Side::Ask, 10100, 5);

  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.depth(Side::Ask), 1);
}

TEST_F(OrderBookTest,
       quantityAt_shouldReturnSumOfBidQuantities_whenLevelHasMultipleOrders) {
  addLimit(100, Side::Bid, 10000, 10);
  addLimit(101, Side::Bid, 10000, 5);

  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 15);
}

TEST_F(OrderBookTest,
       quantityAt_shouldRemainIsolatedPerBidLevel_whenBookHasMultipleLevels) {
  addLimit(100, Side::Bid, 10000, 10);
  addLimit(101, Side::Bid, 5000, 5);

  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 10);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 5000), 5);
}

TEST_F(OrderBookTest,
       quantityAt_shouldReturnZero_whenBidLevelIsEmptyInNonEmptyBook) {
  addLimit(100, Side::Bid, 10000, 10);

  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 5000), 0);
}

TEST_F(OrderBookTest,
       quantityAt_shouldReturnZero_whenLevelExistsOnOppositeSide) {
  addLimit(100, Side::Bid, 10000, 10);

  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
}

TEST_F(OrderBookTest,
       orderCount_shouldCountMultipleOrders_whenTheySharePriceLevel) {
  EXPECT_EQ(orderBook.orderCount(), 0);

  addLimit(100, Side::Bid, 10000, 10);
  EXPECT_EQ(orderBook.orderCount(), 1);

  addLimit(101, Side::Bid, 10000, 5);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

TEST_F(OrderBookTest, orderCount_shouldCountOrders_whenTheyRestOnBothSides) {
  EXPECT_EQ(orderBook.orderCount(), 0);

  addLimit(100, Side::Bid, 5000, 5);
  EXPECT_EQ(orderBook.orderCount(), 1);

  addLimit(101, Side::Ask, 10000, 10);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

TEST_F(
    OrderBookTest,
    queries_shouldUpdateAfterTradeRemovesBestLevels_whenBookTransitionsToEmpty) {
  addLimit(100, Side::Bid, 10000, 5);

  const auto trades = addLimit(200, Side::Ask, 10000, 5);

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.depth(Side::Bid), 0);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);
  EXPECT_EQ(orderBook.orderCount(), 0);
}
