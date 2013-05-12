#include <cmath>
#include <iostream>
#include "Market.h"
#include "OrderBook.h"
#include "Order.h"
#include "Distribution.h"
#include "DistributionUniform.h"
#include "MarketMaker.h"

MarketMaker::MarketMaker(Market *a_market, 
			 Distribution *a_ActionTimeDistribution,
			 Distribution *a_OrderVolumeDistribution,
			 Distribution *a_OrderPriceDistribution,
			 double a_buyFrequency,
			 int a_favouriteStockIdentifier,
			 double a_cancelBuyFrequency,
			 double a_cancelSellFrequency,
			 double a_cancelProbability,
			 double a_volumeProportion) : Agent(a_market, MARKET_MAKER, a_favouriteStockIdentifier) {
  m_ActionTimeDistribution = a_ActionTimeDistribution;
  m_OrderVolumeDistribution = a_OrderVolumeDistribution;
  m_OrderPriceDistribution = a_OrderPriceDistribution;
  m_buyFrequency = a_buyFrequency;
  m_OrderTypeDistribution = new DistributionUniform(a_market->getRNG());
  m_cancelBuyFrequency = a_cancelBuyFrequency;
  m_cancelSellFrequency = a_cancelSellFrequency;
  m_sellFrequency = 1.0 - m_cancelBuyFrequency - m_cancelSellFrequency - m_buyFrequency;
  m_cancelProbability = a_cancelProbability;
  m_cancelDistribution = new DistributionUniform(a_market->getRNG());
  m_VolumeProportion = a_volumeProportion;
}

MarketMaker::~MarketMaker() { }

double MarketMaker::getNextActionTime() const {
  return m_ActionTimeDistribution->nextRandom() ;
}

int MarketMaker::getOrderVolume() const {
  return std::max((int)m_OrderVolumeDistribution->nextRandom(),1) ;		
}

int MarketMaker::getOrderVolume(double price, int a_OrderBookId, OrderType orderType) const {
  if (orderType == LIMIT_BUY) {
    int volumeBid = m_linkToMarket->getOrderBook(a_OrderBookId)->getBidQuantity();
    return std::max(int(m_VolumeProportion * volumeBid),5);		
  }
  else {
    int volumeAsk = m_linkToMarket->getOrderBook(a_OrderBookId)->getAskQuantity();
    return std::max(int(m_VolumeProportion * volumeAsk),5);
  }			
}

int MarketMaker::getOrderPrice(int a_OrderBookId, OrderType a_OrderType) const {
  int price;
  double lag = m_OrderPriceDistribution->nextRandom();
  int tickSize = m_linkToMarket->getOrderBook(a_OrderBookId)->getTickSize();
  //int spread = m_linkToMarket->getOrderBook(a_OrderBookId)->getAskPrice() - m_linkToMarket->getOrderBook(a_OrderBookId)->getBidPrice();
  //if(spread >= 2 *  m_linkToMarket->getOrderBook(a_OrderBookId)->getTickSize()){
  if(a_OrderType == LIMIT_BUY) {
    int currentPrice = m_linkToMarket->getOrderBook(a_OrderBookId)->getBidPrice();
    price = currentPrice + tickSize;
  }
  else if(a_OrderType == LIMIT_SELL) {
    int currentPrice = m_linkToMarket->getOrderBook(a_OrderBookId)->getAskPrice();
    price = currentPrice - tickSize;
  }
  else {
    // GESTION DES ERREURS ET EXCEPTIONS	
  }
  return price;
}

OrderType MarketMaker::getOrderType() const {
  double l_orderTypeAlea = m_OrderTypeDistribution->nextRandom();
  if(l_orderTypeAlea < m_cancelBuyFrequency) {
    // std::cout << "CANCEL_BUY" << std::endl;
    return CANCEL_BUY;
  }
  else if(l_orderTypeAlea < (m_cancelBuyFrequency + m_cancelSellFrequency)) {
    // std::cout << "CANCEL_SELL" << std::endl;
    return CANCEL_SELL;
  }
  else if(l_orderTypeAlea < (m_cancelBuyFrequency + m_cancelSellFrequency + m_buyFrequency)) {
    //std::cout << "LIMIT_BUY" << std::endl;
    return LIMIT_BUY;
  }
  else {
    // std::cout << "LIMIT_SELL " << std::endl;
    return LIMIT_SELL ;
  }
}

void MarketMaker::makeAction(int a_OrderBookId, double a_currentTime) {
  //m_linkToMarket->getOrderBook(a_OrderBookId)->cleanOrderBook();
  OrderType thisOrderType = getOrderType();
  
  if(thisOrderType == CANCEL_BUY || thisOrderType == CANCEL_SELL) {

    /* NO Cancellation at the initialisation of the process */
    if (a_currentTime == 0.0) {
      return;
    }
    else {
      if(thisOrderType == CANCEL_BUY) {
	chooseOrdersToBeCanceled(a_OrderBookId,true,a_currentTime);
	return;
      }
      else {
	chooseOrdersToBeCanceled(a_OrderBookId,false,a_currentTime);
	return;
      }
    }
    return;
  }
  int spread = m_linkToMarket->getOrderBook(a_OrderBookId)->getAskPrice() - m_linkToMarket->getOrderBook(a_OrderBookId)->getBidPrice();
		
  if(spread < (2 *  m_linkToMarket->getOrderBook(a_OrderBookId)->getTickSize())) return;

  if(spread >= (2 *  m_linkToMarket->getOrderBook(a_OrderBookId)->getTickSize())) {
    int thisOrderPrice = getOrderPrice(a_OrderBookId, thisOrderType);
    int thisOrderVolume = getOrderVolume(thisOrderPrice, a_OrderBookId, thisOrderType);
    int tickSize = m_linkToMarket->getOrderBook(a_OrderBookId)->getTickSize();
    submitOrder(a_OrderBookId,
		a_currentTime,
		thisOrderVolume,
		thisOrderType,
		thisOrderPrice);

    if (thisOrderType == LIMIT_BUY) {
      submitOrder(a_OrderBookId,
		  a_currentTime,
		  thisOrderVolume,
		  LIMIT_SELL,
		  thisOrderPrice + tickSize);
    }
    
    else if(thisOrderType==LIMIT_SELL) {
      submitOrder(a_OrderBookId,
		  a_currentTime,
		  thisOrderVolume,
		  LIMIT_BUY,
		  thisOrderPrice - tickSize);
    }		
  }
}

void MarketMaker::chooseOrdersToBeCanceled(int a_OrderBookId, bool a_buySide, double a_time) {
  std::map<int,Order> pendingOrdersCopy(m_pendingOrders);
  std::map<int,Order>::iterator iter = pendingOrdersCopy.begin();

  while(iter != pendingOrdersCopy.end()) {
    OrderType thisOrderType = iter->second.getType();
    if((thisOrderType == LIMIT_BUY && a_buySide) || (thisOrderType == LIMIT_SELL && !a_buySide)) {
	double cancelAlea = m_cancelDistribution->nextRandom();
	if(cancelAlea < m_cancelProbability) {
	  submitCancellation(a_OrderBookId,iter->second.getIdentifier(), a_time);
	}
    }
    iter++;
  }
}

void MarketMaker::processInformation() {
  // For exemple, read the market book history and decide to do something within a reaction Sun May 12 13:43:18 2013
}

void MarketMaker::cleanPending() {
  m_pendingOrders.clear();
}
