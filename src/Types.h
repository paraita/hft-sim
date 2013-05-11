/*
 * Types.h
 *
 *  Created on: 23 avr. 2013
 *      Author: "Paraita Wohler <milkyboi@gmail.com"
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <vector>

typedef enum {
  MARKET_SELL,
  MARKET_BUY,
  LIMIT_SELL,
  LIMIT_BUY,
  CANCEL_SELL,
  CANCEL_BUY,
  CLEAR_OB
} OrderType;

typedef enum {
  ALIVE,
  EXECUTED,
  PARTIALLY_EXECUTED,
  CANCELED
} OrderState;

typedef enum {
  DEFAULT,
  LIQUIDITY_PROVIDER,
  LIQUIDITY_PROVIDER_UC,
  NOISE_TRADER,
  LARGE_ORDER_TRADER,
  TREND_FOLLOWER,
  FUNDAMENTAL_VALUE_TRADER,
  MARKET_MAKER
} AgentType;

class ExecutionHistory {

 public:

  ExecutionHistory(double _time, int _price, int _quantity) {
    executedTime = _time;
    executedPrice = _price;
    executedQuantity = _quantity;
  }

  double executedTime;
  int executedPrice;
  int executedQuantity;

};

class LimitOrders {

 public:

  LimitOrders(int _bid, int _bidQ, int _ask, int _askQ) {
    bid = _bid;
    bidQ = _bidQ;
    ask = _ask;
    askQ = _askQ;
  }

  int bid;
  int bidQ;
  int ask;
  int askQ;

};

class OrderBookHistory {

 public:
  OrderBookHistory(double _datexl,
		   double _timeStamp,
		   std::vector<LimitOrders> _limitOrders,
		   int _last,
		   int _lastQ) {
    datexl = _datexl;
    timestamp = _timeStamp;
    limitOrders = _limitOrders;
    last = _last;
    lastQ = _lastQ;
  }

  double datexl;
  double timestamp;
  std::vector<LimitOrders> limitOrders;
  int last;
  int lastQ;

};

#endif /* TYPES_H_ */
