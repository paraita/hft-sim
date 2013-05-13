#include <iostream>
#include <cstdlib>
#include "Market.h"
#include "Agent.h"
#include "Order.h"
#include "OrderBook.h"

Agent::Agent(Market *a_market, AgentType a_Type, int a_favouriteStockId) {
  linkToMarket = a_market;
  type = a_Type;
  cashPosition = 0;
  favouriteStockId = a_favouriteStockId;
  fees = 0.0;
  identifier = 0;
}

Agent::~Agent() { }

int Agent::getFavouriteStock() {
  return favouriteStockId;
}

AgentType Agent::getAgentType() const { 
  return type;
}

void Agent::setIdentifier(int a_identifier) {
  identifier = a_identifier;
}

void Agent::submitOrder(int a_asset,
			double a_time,
			int a_volume,
			OrderType a_type,
			int a_price) {
  if(a_volume != 0) {
    int l_orderIdentifier = linkToMarket->getOrderIdentifier();
    Order l_order(a_asset,
		  identifier,
		  a_time,
		  a_price,
		  a_volume,
		  a_type,
		  l_orderIdentifier);
    passedOrders[l_orderIdentifier] = l_order;
    pendingOrders[l_orderIdentifier] = l_order;
    linkToMarket->pushOrder(l_order);
  }
}

void Agent::submitCancellation(int a_OrderBookId, int a_orderIdentifier, double a_time) {
  linkToMarket->getOrderBook(a_OrderBookId)->processCancellation(identifier, a_orderIdentifier, a_time);
}
void Agent::notifyExecution(int a_orderIdentifier,
			    double a_time,
			    int a_price) {
  Order *l_order = &passedOrders[a_orderIdentifier];
  l_order->setState(EXECUTED);
	
  // Remove from pending orders list
  std::map<int, Order>::iterator iter = pendingOrders.find(a_orderIdentifier) ;
  if(iter == pendingOrders.end()){
    std::cout << "a_orderIdentifier not found in Agent::notifyExecution." << std::endl;
    exit(1);
  }
  pendingOrders.erase(iter);

  l_order->executionHistory.push_back(ExecutionHistory(a_time, l_order->volume, a_price));
  updateStockNumber(l_order, l_order->volume);
  processCashPosition(a_time, l_order->volume, a_price, l_order->type);
  l_order->volume = 0;
}

void Agent::notifyPartialExecution(int a_orderIdentifier, double a_time, int a_volume, int a_price) {
  Order *l_order = &passedOrders[a_orderIdentifier];
  l_order->setState(PARTIALLY_EXECUTED);
	
  l_order->executionHistory.push_back(ExecutionHistory(a_time,a_volume,a_price));
  l_order->setVolume(l_order->getVolume() - a_volume);
	
  updateStockNumber(l_order,a_volume);
  processCashPosition(a_time, a_volume, a_price, l_order->type);
}

void Agent::notifyCancellation(int a_orderIdentifier, double a_time) {
  Order *l_order = &passedOrders[a_orderIdentifier];
  l_order->state = CANCELED;

  // Remove from pending orders list
  std::map<int, Order>::iterator iter = pendingOrders.find(a_orderIdentifier) ;
  if(iter == pendingOrders.end()){
    std::cout << "a_orderIdentifier not found in Agent::notifyCancellation." << std::endl ;
    exit(1);
  }
  pendingOrders.erase(iter) ;
	
  l_order->executionHistory.push_back(ExecutionHistory(a_time, -1, -1));
}

void Agent::processCashPosition(double a_time, int a_volume, int a_price, const OrderType& a_type) {
  int l_cashFlow = a_volume * a_price;
  int l_type;
  switch (a_type) {
    case LIMIT_SELL:
    case MARKET_SELL:
      l_type = 1;
      break;
    case LIMIT_BUY:
    case MARKET_BUY:
      l_type =-1;
      break;
    default:// Nothing to do with other order types
      break;
    }
	
  cashPosition += (l_type * l_cashFlow);
  cashPositionHistory.push_back(std::make_pair(a_time, cashPosition));
}

void Agent::notifyAgentOfMarketEvent() {
  processInformation();
}

void Agent::initAssetQuantity(int a_stockIdentifier) {
  stockQuantity[a_stockIdentifier] = 0;
}

int Agent::getStockQuantity(int a_stockIdentifier) {
  return stockQuantity[a_stockIdentifier];
}

void Agent::updateStockNumber(Order *a_order,int a_executedVolume) {
  switch(a_order->type)
    {
    case LIMIT_SELL:
    case MARKET_SELL:
      stockQuantity[a_order->asset] -= a_executedVolume;
      break;
    case LIMIT_BUY:
    case MARKET_BUY:
      stockQuantity[a_order->asset] += a_executedVolume;
      break;
    default:// Nothing to do with other order types
      break;
    }
}

bool Agent::isMine(Order a_order) const {
  bool l_mine = false;
  if (a_order.owner == identifier)
    {
      l_mine = true;
    }
  return l_mine;
}

void Agent::chargeMarketFees(double a_fee) {
  fees += a_fee;
}

double Agent::getNetCashPosition() {
  return (cashPosition - fees);
}

int Agent::getTargetedStock() {
  return getFavouriteStock();
}

double Agent::getFeeRate(OrderType a_orderType) {
  return 0.0;
}

std::map<int,Order> * Agent::getPendingOrders() {
  return &pendingOrders;
}

