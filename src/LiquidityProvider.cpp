#include "Market.h"
#include "OrderBook.h"
#include "Order.h"
#include "Distribution.h"
#include "DistributionUniform.h"
#include "LiquidityProvider.h"
#include "LiquidityProvider.h"
#include <cmath>
#include <iostream>

LiquidityProvider::LiquidityProvider(
	Market * a_market, 
	Distribution * a_ActionTimeDistribution,
	Distribution * a_OrderVolumeDistribution,
	Distribution * a_OrderPriceDistribution,
	double a_buyFrequency,
	int a_favouriteStockIdentifier,
	double a_cancelBuyFrequency,
	double a_cancelSellFrequency,
	double a_cancelProbability
	):Agent(a_market,LIQUIDITY_PROVIDER, a_favouriteStockIdentifier)
{
	m_ActionTimeDistribution = a_ActionTimeDistribution ;
	m_OrderVolumeDistribution = a_OrderVolumeDistribution ;
	m_OrderPriceDistribution = a_OrderPriceDistribution;
	m_buyFrequency = a_buyFrequency ;
	m_OrderTypeDistribution = new DistributionUniform(a_market->getRNG()) ;
	m_cancelBuyFrequency = a_cancelBuyFrequency;
	m_cancelSellFrequency = a_cancelSellFrequency;
	m_sellFrequency = 1.0 - m_cancelBuyFrequency - m_cancelSellFrequency - m_buyFrequency;
	m_cancelProbability = a_cancelProbability ;
	m_cancelDistribution = new DistributionUniform(a_market->getRNG()) ;

}
LiquidityProvider::~LiquidityProvider() 
{

}
double LiquidityProvider::getNextActionTime() const
{
	return m_ActionTimeDistribution->nextRandom() ;
}
int LiquidityProvider::getOrderVolume() const
{
	return std::max((int)m_OrderVolumeDistribution->nextRandom(),1) ;		
}

int LiquidityProvider::getOrderVolume(double price, int a_OrderBookId, OrderType orderType ) const
{
	int slotMax = 20;
	if (orderType==LIMIT_BUY){
		int currentPriceAsk = m_linkToMarket->getOrderBook(a_OrderBookId)->getAskPrice() ;

		if ( price < currentPriceAsk && price>(currentPriceAsk-0.25*slotMax ) ) {
			//int volume = std::min((int)m_OrderVolumeDistribution->nextRandom(),100);
			int volume = (int)m_OrderVolumeDistribution->nextRandom();
			return std::max(volume , 1  ) ;
		}
		else{
			int slotNumber;
			slotNumber = (int)(currentPriceAsk- price)/(m_linkToMarket->getOrderBook(a_OrderBookId)->getTickSize());		
			DistributionUniform * uniformVolumeDistribution = new DistributionUniform(m_linkToMarket->getRNG(),0,(70*6)/(slotNumber^(3/2)));
			int volume = std::max((int)uniformVolumeDistribution->nextRandom(),1) ;
			delete uniformVolumeDistribution;
			return volume;
		}
	}
	else{
		int currentPriceBid = m_linkToMarket->getOrderBook(a_OrderBookId)->getBidPrice() ;

		if ( price > currentPriceBid && price<(currentPriceBid+0.25*slotMax ) ) {
			//int volume = std::min((int)m_OrderVolumeDistribution->nextRandom(),100);
			int volume = (int)m_OrderVolumeDistribution->nextRandom();
			return std::max(volume , 1  ) ;
		}
		else{
			int slotNumber;
			slotNumber = (int)(price-currentPriceBid)/(m_linkToMarket->getOrderBook(a_OrderBookId)->getTickSize());		
			DistributionUniform * uniformVolumeDistribution = new DistributionUniform(m_linkToMarket->getRNG(),0,(70*6)/(slotNumber^(3/2)));
			int volume = std::max((int)uniformVolumeDistribution->nextRandom(),1) ;
			delete uniformVolumeDistribution;
			return volume;		
		}

	}

}

int LiquidityProvider::getOrderPrice(int a_OrderBookId, OrderType a_OrderType) const
{
	int price ;
	double lag = m_OrderPriceDistribution->nextRandom() ;
	int tickSize = m_linkToMarket->getOrderBook(a_OrderBookId)->getTickSize() ;
	if(a_OrderType==LIMIT_BUY)
	{
		int currentPrice = m_linkToMarket->getOrderBook(a_OrderBookId)->getAskPrice() ;
		price = currentPrice - (((int)lag)*tickSize) - tickSize ;
	}
	else if(a_OrderType==LIMIT_SELL)
	{
		int currentPrice = m_linkToMarket->getOrderBook(a_OrderBookId)->getBidPrice() ;
		price = currentPrice + (((int)lag)*tickSize) + tickSize ;
	}
	else
	{
		// GESTION DES ERREURS ET EXCEPTIONS	
	}
	return price ;
}
OrderType LiquidityProvider::getOrderType() const
{
	double l_orderTypeAlea = m_OrderTypeDistribution->nextRandom();
	if(l_orderTypeAlea<m_cancelBuyFrequency)
	{
		//		std::cout << "CANCEL_BUY" << std::endl ;
		return CANCEL_BUY;
	}
	else if(l_orderTypeAlea<m_cancelBuyFrequency + m_cancelSellFrequency)
	{
		//		std::cout << "CANCEL_SELL" << std::endl ;
		return CANCEL_SELL;
	}
	else if(l_orderTypeAlea<m_cancelBuyFrequency + m_cancelSellFrequency + m_buyFrequency)
	{
		//std::cout << "LIMIT_BUY" << std::endl ;
		return LIMIT_BUY ;
	}
	else
	{
		//		std::cout << "LIMIT_SELL " << std::endl ;
		return LIMIT_SELL ;
	}
}

void LiquidityProvider::makeAction(int a_OrderBookId, double a_currentTime)
{

	OrderType thisOrderType = getOrderType() ;
	if(thisOrderType == CANCEL_BUY ||thisOrderType == CANCEL_SELL)
	{
		if (a_currentTime == 0.0)// NO Cancellation at the initialisation of the process
		{
			return;
		}
		else
		{
			if(thisOrderType == CANCEL_BUY)
			{
				chooseOrdersToBeCanceled(a_OrderBookId,true,a_currentTime);
				return;
			}
			else
			{
				chooseOrdersToBeCanceled(a_OrderBookId,false,a_currentTime);
				return;
			}
		}
	}

	int thisOrderPrice = getOrderPrice(a_OrderBookId, thisOrderType) ;

	int thisOrderVolume = getOrderVolume(thisOrderPrice, a_OrderBookId, thisOrderType);
	submitOrder(
		a_OrderBookId, 
		a_currentTime,
		thisOrderVolume,
		thisOrderType,
		thisOrderPrice
		);	
}

void LiquidityProvider::makeSellAction(int a_OrderBookId, double a_currentTime)
{

	OrderType thisOrderType = LIMIT_SELL;
	int thisOrderPrice = getOrderPrice(a_OrderBookId, thisOrderType) ;

	int thisOrderVolume = getOrderVolume(thisOrderPrice, a_OrderBookId, thisOrderType);
	submitOrder(
		a_OrderBookId, 
		a_currentTime,
		thisOrderVolume,
		thisOrderType,
		thisOrderPrice
		);	
}

void LiquidityProvider::makeBuyAction(int a_OrderBookId, double a_currentTime)
{

	OrderType thisOrderType = LIMIT_BUY;
	int thisOrderPrice = getOrderPrice(a_OrderBookId, thisOrderType) ;

	int thisOrderVolume = getOrderVolume(thisOrderPrice, a_OrderBookId, thisOrderType);
	submitOrder(
		a_OrderBookId, 
		a_currentTime,
		thisOrderVolume,
		thisOrderType,
		thisOrderPrice
		);	
}

void LiquidityProvider::chooseOrdersToBeCanceled(int a_OrderBookId, bool a_buySide, double a_time)
{
	mtx_.lock();
	concurrency::concurrent_unordered_map<int,Order> pendingOrdersCopy(m_pendingOrders) ;
	concurrency::concurrent_unordered_map<int,Order>::iterator iter = pendingOrdersCopy.begin();
	mtx_.unlock();

	while(iter!=pendingOrdersCopy.end()){
		OrderType thisOrderType = iter->second.getType() ;

		if(m_linkToMarket->getOrderBook(a_OrderBookId)->getDistanceToBestOppositeQuote( iter->second.getPrice())>18){
			if(thisOrderType == LIMIT_BUY)
				submitCancellation(a_OrderBookId,iter->second.getIdentifier(), iter->second.getPrice(), a_time, CANCEL_BUY) ;
			else
				submitCancellation(a_OrderBookId,iter->second.getIdentifier(), iter->second.getPrice(),  a_time, CANCEL_SELL) ;
		}else {
			if((thisOrderType==LIMIT_BUY && a_buySide)||(thisOrderType==LIMIT_SELL && !a_buySide))
			{
				OrderType type = a_buySide ? CANCEL_BUY : CANCEL_SELL;
				double cancelAlea = m_cancelDistribution->nextRandom() ;
				if(cancelAlea<m_cancelProbability){
					submitCancellation(a_OrderBookId,iter->second.getIdentifier(),iter->second.getPrice(), a_time, type) ;
				}
			}
		}
		iter++ ;
	}
}
void LiquidityProvider::processInformation()
{
	// For exemple, read the market book history and decide to do something within a reaction time

}

void LiquidityProvider::printPending()
{
	mtx_.lock();
	concurrency::concurrent_unordered_map<int,Order> pendingOrdersCopy(m_pendingOrders) ;
	concurrency::concurrent_unordered_map<int,Order>::iterator iter = pendingOrdersCopy.begin();
	mtx_.unlock();

	while(iter!=pendingOrdersCopy.end()){
		Order thisOrder = iter->second;

		std::cout << "price: " << thisOrder.getPrice() << std::endl;
		std::cout << "volume: " << thisOrder.getVolume() << std::endl;

		iter++ ;
	}

}

