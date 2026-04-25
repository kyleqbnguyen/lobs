#include "fixture.h"

TEST_F(OrderBookTest, modifyOrder_shouldReturnTrue_whenOrderExists) {
  addLimit(100, Side::Bid, 10000, 10);

  EXPECT_TRUE(orderBook.modifyOrder(100, 5));
}

TEST_F(OrderBookTest, modifyOrder_shouldReturnFalse_whenOrderDoesNotExist) {
  EXPECT_FALSE(orderBook.modifyOrder(999, 5));
}

TEST_F(OrderBookTest,
       modifyOrder_shouldUpdateLevelQuantity_whenBidQuantityReduced) {
  addLimit(100, Side::Bid, 10000, 10);

  orderBook.modifyOrder(100, 4);

  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 4);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       modifyOrder_shouldUpdateOnlyTargetOrder_whenMultipleOrdersAtLevel) {
  addLimit(100, Side::Ask, 10000, 10);
  addLimit(101, Side::Ask, 10000, 6);

  orderBook.modifyOrder(100, 3);

  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 9);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

TEST_F(OrderBookTest,
       modifyOrder_shouldPreserveTimePriority_whenQuantityReduced) {
  addLimit(100, Side::Ask, 10000, 10);
  addLimit(101, Side::Ask, 10000, 5);

  orderBook.modifyOrder(100, 3);

  const auto trades = addLimit(200, Side::Bid, 10000, 4);

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].quantity, 3);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].quantity, 1);
}

TEST_F(OrderBookTest,
       modifyOrder_shouldNotAffectBestPrice_whenQuantityReduced) {
  addLimit(100, Side::Bid, 10000, 10);

  orderBook.modifyOrder(100, 4);

  EXPECT_EQ(orderBook.bestBid(), 10000);
}

TEST_F(OrderBookTest,
       modifyOrder_shouldNotAffectOppositeSide_whenBidIsModified) {
  addLimit(100, Side::Bid, 10000, 10);
  addLimit(101, Side::Ask, 10100, 5);

  orderBook.modifyOrder(100, 3);

  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 5);
}
