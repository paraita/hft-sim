#include <cmath>
#include <iostream>
#include "Market.h"
#include "OrderBook.h"
#include "Order.h"
#include "Distribution.h"
#include "DistributionUniform.h"
#include "LiquidityProvider.h"
#include "LiquidityProvider.h"

LiquidityProvider::LiquidityProvider(Market *a_market, 
				     Distribution *a_ActionTimeDistribution,
				     Distribution *a_OrderVolumeDistribution,
				     Distribution *a_OrderPriceDistribution,
				     double a_buyFrequency,
				     int a_favouriteStockIdentifier,
				     double a_cancelBuyFrequency,
				     double a_cancelSellFrequency,
				     double a_cancelProbability) : Agent(a_market,LIQUIDITY_PROVIDER, a_favouriteStockIdentifier) {
  actionTimeDistribution = a_ActionTimeDistribution;
  orderVolumeDistribution = a_OrderVolumeDistribution;
  orderPriceDistribution = a_OrderPriceDistribution;
  buyFrequency = a_buyFrequency;
  orderTypeDistribution = new DistributionUniform(a_market->getRNG());
  cancelBuyFrequency = a_cancelBuyFrequency;
  cancelSellFrequency = a_cancelSellFrequency;
  sellFrequency = 1.0 - cancelBuyFrequency - cancelSellFrequency - buyFrequency;
  cancelProbability = a_cancelProbability;
  cancelDistribution = new DistributionUniform(a_market->getRNG());
}

LiquidityProvider::~LiquidityProvider() { }

double LiquidityProvider::getNextActionTime() const {
  return actionTimeDistribution->nextRandom() ;
}

int LiquidityProvider::getOrderVolume() const {
  return std::max((int) orderVolumeDistribution->nextRandom(), 1) ;
}

int LiquidityProvider::getOrderVolume(double price, int a_OrderBookId, OrderType orderType) const {
  int slotMax = 20;
  if (orderType == LIMIT_BUY){
    int currentPriceAsk = linkToMarket->getOrderBook(a_OrderBookId)->getAskPrice();

    if (price < currentPriceAsk && price>(currentPriceAsk - 0.25 * slotMax)) {
      int volume = std::min((int) orderVolumeDistribution->nextRandom(), 100);
      return std::max(volume, 1);
    }
    else {
      int slotNumber;
      slotNumber = (int) (currentPriceAsk- price) / (linkToMarket->getOrderBook(a_OrderBookId)->getTickSize());
      DistributionUniform *uniformVolumeDistribution = new DistributionUniform(linkToMarket->getRNG(),
									       0,
									       (70 * 6) / (slotNumber^(3 / 2)));
      int volume = std::max((int) uniformVolumeDistribution->nextRandom(), 1);
      delete uniformVolumeDistribution;
      return volume;
    }
  }
  else {
    int currentPriceBid = linkToMarket->getOrderBook(a_OrderBookId)->getBidPrice() ;

    if (price > currentPriceBid && price<(currentPriceBid + 0.25 * slotMax)) {
      int volume = std::min((int) orderVolumeDistribution->nextRandom(), 100);
      return std::max(volume, 1);
    }
    else {
      int slotNumber;
      slotNumber = (int) (price-currentPriceBid) / (linkToMarket->getOrderBook(a_OrderBookId)->getTickSize());
      DistributionUniform *uniformVolumeDistribution = new DistributionUniform(linkToMarket->getRNG(),
									       0,
									       (70 * 6) / (slotNumber^(3 / 2)));
      int volume = std::max((int)uniformVolumeDistribution->nextRandom(), 1);
      delete uniformVolumeDistribution;
      return volume;		
    }
  }			
}

int LiquidityProvider::getOrderPrice(int a_OrderBookId, OrderType a_OrderType) const {
  int price;
  double lag = orderPriceDistribution->nextRandom();
  int tickSize = linkToMarket->getOrderBook(a_OrderBookId)->getTickSize();
  if(a_OrderType == LIMIT_BUY) {
    int currentPrice = linkToMarket->getOrderBook(a_OrderBookId)->getAskPrice();
    price = currentPrice - (((int) lag) * tickSize) - tickSize;
  }
  else if(a_OrderType==LIMIT_SELL) {
    int currentPrice = linkToMarket->getOrderBook(a_OrderBookId)->getBidPrice();
    price = currentPrice + (((int) lag) * tickSize) + tickSize;
  }
  else {
    // GESTION DES ERREURS ET EXCEPTIONS	
  }
  return price;
}

OrderType LiquidityProvider::getOrderType() const {
  double l_orderTypeAlea = orderTypeDistribution->nextRandom();
  if(l_orderTypeAlea < cancelBuyFrequency) {
    /* std::cout << "CANCEL_BUY" << std::endl; */
    return CANCEL_BUY;
  }
  else if(l_orderTypeAlea<cancelBuyFrequency + cancelSellFrequency) {
    /* std::cout << "CANCEL_SELL" << std::endl; */
    return CANCEL_SELL;
  }
  else if(l_orderTypeAlea < (cancelBuyFrequency + cancelSellFrequency + buyFrequency)) {
    /* std::cout << "LIMIT_BUY" << std::endl; */
    return LIMIT_BUY ;
  }
  else {
    /* std::cout << "LIMIT_SELL " << std::endl; */
    return LIMIT_SELL ;
  }
}

void LiquidityProvider::makeAction(int a_OrderBookId, double a_currentTime) {
  makeAction(a_OrderBookId, a_currentTime,true);
}
void LiquidityProvider::makeAction(int a_OrderBookId, double a_currentTime, bool init) {
  //m_linkToMarket->getOrderBook(a_OrderBookId)->cleanOrderBook();
  //OrderType thisOrderType = getOrderType() ;
  //if(thisOrderType == CANCEL_BUY ||thisOrderType == CANCEL_SELL)
  //{
  //	if (a_currentTime == 0.0)// NO Cancellation at the initialisation of the process
  //	{
  //		return;
  //	}
  //	else
  //	{
  //		if(thisOrderType == CANCEL_BUY)
  //		{
  //			chooseOrdersToBeCanceled(a_OrderBookId,true,a_currentTime);
  //			return;
  //		}
  //		else
  //		{
  //			chooseOrdersToBeCanceled(a_OrderBookId,false,a_currentTime);
  //			return;
  //		}
  //	}
  //}
		
  //	int thisOrderPrice = getOrderPrice(a_OrderBookId, thisOrderType) ;
  //	int thisOrderVolume = getOrderVolume(thisOrderPrice, a_OrderBookId, thisOrderType) ;
  ////	int thisOrderVolume = getOrderVolume() ;
  //	submitOrder(
  //		a_OrderBookId, a_currentTime,
  //		thisOrderVolume,
  //		thisOrderType,
  //		thisOrderPrice
  //	);
  //	std::cout<<"LIMIT order!!!"<<std::endl;
  //std::cout<<"LIQUIDITY ORDER"<<std::endl;

  int tickSize = linkToMarket->getOrderBook(a_OrderBookId)->getTickSize();
  int i;
  int end;
  if (init) {
    i = 1;
    end = 20;
  }
  else {
    i = 0;
    end = 19;
  }
		
  DistributionUniform *u = new DistributionUniform(linkToMarket->getRNG(), 0, 1);
  for (; i <= end; i++) {
    if( u->nextRandom() > 0.5) {
      int currentPriceAsk = linkToMarket->getOrderBook(a_OrderBookId)->getAskPrice();
      //std::cout<<"current Ask : "<<currentPriceAsk<<std::endl;
      //std::cout<<"current Bid : "<<currentPriceBid<<std::endl;

      int buyPrice = currentPriceAsk - (i) * tickSize;
      int buyOrderVolume = getOrderVolume(buyPrice, a_OrderBookId, LIMIT_BUY);
      submitOrder(a_OrderBookId,
		  a_currentTime,
		  buyOrderVolume,
		  LIMIT_BUY,
		  buyPrice);
      int currentPriceBid = linkToMarket->getOrderBook(a_OrderBookId)->getBidPrice();
      int sellPrice = currentPriceBid+(i)*tickSize;
      int sellOrderVolume = getOrderVolume(sellPrice, a_OrderBookId, LIMIT_SELL);
      submitOrder(a_OrderBookId,
		  a_currentTime,
		  sellOrderVolume,
		  LIMIT_SELL,
		  sellPrice);
    }
    else {
      int currentPriceBid = linkToMarket->getOrderBook(a_OrderBookId)->getBidPrice();
      int sellPrice = currentPriceBid+(i)*tickSize;
      int sellOrderVolume = getOrderVolume(sellPrice, a_OrderBookId, LIMIT_SELL);
      submitOrder(a_OrderBookId,
		  a_currentTime,
		  sellOrderVolume,
		  LIMIT_SELL,
		  sellPrice);
      int currentPriceAsk = linkToMarket->getOrderBook(a_OrderBookId)->getAskPrice();
      /* std::cout<<"current Ask : "<<currentPriceAsk<<std::endl; */
      /* std::cout<<"current Bid : "<<currentPriceBid<<std::endl; */

      int buyPrice = currentPriceAsk - (i) * tickSize;
      int buyOrderVolume = getOrderVolume(buyPrice, a_OrderBookId, LIMIT_BUY);
      submitOrder(a_OrderBookId,
		  a_currentTime,
		  buyOrderVolume,
		  LIMIT_BUY,
		  buyPrice);
    }
  }
  delete u;
}

void LiquidityProvider::chooseOrdersToBeCanceled(int a_OrderBookId, bool a_buySide, double a_time) {
  std::map<int,Order> pendingOrdersCopy(pendingOrders);
  std::map<int,Order>::iterator iter = pendingOrdersCopy.begin();
  
  while(iter != pendingOrdersCopy.end()) {
    OrderType thisOrderType = iter->second.getType();
    if((thisOrderType == LIMIT_BUY && a_buySide) || (thisOrderType == LIMIT_SELL && !a_buySide)) {
      double cancelAlea = cancelDistribution->nextRandom();
      if(cancelAlea < cancelProbability) {
	submitCancellation(a_OrderBookId, iter->second.getIdentifier(), a_time);
      }
    }
    iter++;
  }
}

void LiquidityProvider::processInformation() {
  // For exemple, read the market book history and decide to do something within a reaction time
}

void LiquidityProvider::cleanPending() {
  pendingOrders.clear();
}

