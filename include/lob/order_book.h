#pragma once

#include "lob/types.h"

#include <cstddef>
#include <map>
#include <optional>
#include <unordered_map>

class OrderBook {
public:
  Trades addOrder(Order order);

  // bool cancelOrder(OrderId orderId);
  //
  // bool modifyOrder(OrderId orderId, Quantity quantity);

  std::optional<Price> bestBid();

  std::optional<Price> bestAsk();

  std::size_t getBookDepth(Side side);

  Quantity getQuantityAt(Side side, Price price);

  std::size_t getOrderBookSize();

private:
  void matchOrder(Order& order, Trades& trades);

  // bool canFillFully(const Order& order);

  std::map<Price, LevelInfo> bids_;
  std::map<Price, LevelInfo> asks_;

  std::unordered_map<OrderId, Order> orders_;
};
