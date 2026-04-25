#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest, addOrder_shouldReturnNoTrades_whenOrderRests) {
  const auto trades = addLimit(100, Side::Bid, 10000, 10);

  EXPECT_TRUE(trades.empty());
}

TEST_F(OrderBookTest,
       addOrder_shouldPreserveExistingSideState_whenOppositeSideOrderRests) {
  addLimit(100, Side::Bid, 10000, 10);

  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 10);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 1);

  addLimit(101, Side::Ask, 15000, 7);

  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), 15000);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.depth(Side::Ask), 1);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 10);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 15000), 7);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 15000), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

TEST_F(OrderBookTest, addOrder_shouldRestBidWithoutTrades_whenBidDoesNotCross) {
  addLimit(100, Side::Ask, 10100, 10);

  const auto trades = addLimit(200, Side::Bid, 10000, 5);

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 10);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

