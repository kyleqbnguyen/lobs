#pragma once

#include "lob/map_order_book.h"
#include "lob/types.h"

#include <gtest/gtest.h>

class OrderBookTest : public ::testing::Test {
protected:
  MapOrderBook orderBook{};

  Trades addLimit(OrderId id, Side side, Price price, Quantity quantity,
                  TimeInForce timeInForce = TimeInForce::GTC) {
    return orderBook.addOrder({.id = id,
                               .side = side,
                               .type = OrderType::Limit,
                               .timeInForce = timeInForce,
                               .price = price,
                               .quantity = quantity});
  }

  Trades addMarket(OrderId id, Side side, Quantity quantity,
                   TimeInForce timeInForce = TimeInForce::IOC) {
    return orderBook.addOrder({.id = id,
                               .side = side,
                               .type = OrderType::Market,
                               .timeInForce = timeInForce,
                               .price = 0,
                               .quantity = quantity});
  }
};
