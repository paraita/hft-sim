#include "Market.h"
#include "OrderBook.h"
#include "Distribution.h"
#include "NoiseTrader.h"
#include <cmath>
#include <iostream>

NoiseTrader::NoiseTrader(
		Market * a_market, 
		Distribution * a_ActionTimeDistribution,
		Distribution * a_OrderTypeDistribution,
		Distribution * a_OrderVolumeDistribution,
		double a_buyFrequency,
		int a_favouriteStockIdentifier
		):Agent(a_market,NOISE_TRADER, a_favouriteStockIdentifier)
{
	m_ActionTimeDistribution = a_ActionTimeDistribution ;
	m_OrderTypeDistribution = a_OrderTypeDistribution ;
	m_OrderVolumeDistribution = a_OrderVolumeDistribution ;
	m_buyFrequency = a_buyFrequency;
	m_sellFrequency = 1.0 - m_buyFrequency;
	
}
NoiseTrader::~NoiseTrader()
{

}

double NoiseTrader::getNextActionTime() const
{
	return m_ActionTimeDistribution->nextRandom() ;
}

OrderType NoiseTrader::getOrderType() const
{
	if(m_OrderTypeDistribution->nextRandom()<m_buyFrequency)
	{
//		std::cout << "MARKET_BUY " << std::endl ;
		return MARKET_BUY ;
	}
	else
	{
//		std::cout << "MARKET_SELL " << std::endl ;
		return MARKET_SELL ;
	}
}

int NoiseTrader::getOrderVolume() const
{
//	std::cout<<"market volume simulated"<<std::endl;
	return (int)m_OrderVolumeDistribution->nextRandom();		
}

void NoiseTrader::makeAction(int a_OrderBookId, double a_currentTime)
{

	//std::cout<<"market order!!!"<<std::endl;
	OrderType thisOrderType = getOrderType() ;
	int thisOrderVolume = getOrderVolume() ;

	//std::cout << "ask quant:" << m_linkToMarket->getOrderBook(a_OrderBookId)->getAskQ

	submitOrder(
		a_OrderBookId, a_currentTime,
		thisOrderVolume,
		thisOrderType
	) ;	
}
void NoiseTrader::processInformation()
{
	// For exemple, read the market book history and decide to do something within a reaction time
}
int NoiseTrader::getOrderPrice (int a_OrderBookId, OrderType a_OrderType) const
{
	return -1;// A noise trader does not specify a price
}

void NoiseTrader::submitCancellation(int a_OrderBookId,bool a_cancelBuy) const
{

}
