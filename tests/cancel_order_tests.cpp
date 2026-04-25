#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest, cancelOrder_shouldReturnTrue_whenOrderExists) {
  addLimit(100, Side::Bid, 10000, 10);

  EXPECT_TRUE(orderBook.cancelOrder(100));
}

TEST_F(OrderBookTest, cancelOrder_shouldReturnFalse_whenOrderDoesNotExist) {
  EXPECT_FALSE(orderBook.cancelOrder(999));
}

TEST_F(OrderBookTest, cancelOrder_shouldReturnFalse_whenOrderAlreadyCancelled) {
  addLimit(100, Side::Bid, 10000, 10);

  orderBook.cancelOrder(100);

  EXPECT_FALSE(orderBook.cancelOrder(100));
}

TEST_F(OrderBookTest, cancelOrder_shouldEmptyBook_whenOnlyBidIsCancelled) {
  addLimit(100, Side::Bid, 10000, 10);

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.depth(Side::Bid), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldUpdateBestBid_whenBestBidLevelIsCancelled) {
  addLimit(100, Side::Bid, 10000, 10);
  addLimit(101, Side::Bid, 9900, 5);

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.bestBid(), 9900);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldUpdateBestAsk_whenBestAskLevelIsCancelled) {
  addLimit(100, Side::Ask, 10000, 10);
  addLimit(101, Side::Ask, 10100, 5);

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.depth(Side::Ask), 1);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldNotDecrementDepth_whenOtherOrdersRemainAtLevel) {
  addLimit(100, Side::Bid, 10000, 10);
  addLimit(101, Side::Bid, 10000, 5);

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldReduceLevelQuantity_whenOneOfMultipleOrdersCancelled) {
  addLimit(100, Side::Ask, 10000, 6);
  addLimit(101, Side::Ask, 10000, 4);

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 4);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldPreserveFifo_whenLeadingOrderAtLevelIsCancelled) {
  addLimit(100, Side::Ask, 10000, 5);
  addLimit(101, Side::Ask, 10000, 7);
  addLimit(102, Side::Ask, 10000, 3);

  orderBook.cancelOrder(100);

  const auto trades = addLimit(200, Side::Bid, 10000, 8);

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].passiveId, 101);
  EXPECT_EQ(trades[0].quantity, 7);
  EXPECT_EQ(trades[1].passiveId, 102);
  EXPECT_EQ(trades[1].quantity, 1);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldNotAffectOppositeSide_whenBidIsCancelled) {
  addLimit(100, Side::Bid, 10000, 10);
  addLimit(101, Side::Ask, 10100, 5);

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}
