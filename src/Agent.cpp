#include "Market.h"
#include "Agent.h"
#include "Order.h"
#include "OrderBook.h"
#include <iostream>
#include <cstdlib>

Agent::Agent(Market *a_market, AgentType a_Type, int a_favouriteStockId)
{
	m_linkToMarket	= a_market;
	m_Type = a_Type ;
	m_cashPosition = 0;
	m_favouriteStockId = a_favouriteStockId;
	m_fees = 0.0;
}
Agent::~Agent()
{

}
int Agent::getFavouriteStock()
{
	return m_favouriteStockId;
}
AgentType Agent::getAgentType()const
{ 
	return m_Type ; 
}
void Agent::setIdentifier(int a_identifier)
{
	m_identifier = a_identifier;	
}
void Agent::submitOrder(int a_asset , double a_time ,
						int	a_volume , OrderType a_type, int a_price, int priority )
{
	if(a_volume!=0){
		int l_orderIdentifier = m_linkToMarket->getOrderIdentifier();
		Order l_order(a_asset , m_identifier ,a_time , a_price,
			a_volume ,a_type ,
			l_orderIdentifier, ALIVE);
		
		m_passedOrders[l_orderIdentifier] = l_order;
		mtx_.lock();
		m_pendingOrders[l_orderIdentifier] = l_order;
		mtx_.unlock();
		m_linkToMarket->pushOrder(l_order);
	}
}

void Agent::submitCancellation(int a_OrderBookId,int a_orderIdentifier, int a_price,  double a_time, OrderType type)
{
	Order l_order(a_OrderBookId , m_identifier ,a_time , a_price,
		0 ,type ,
		a_orderIdentifier, ALIVE);

	m_linkToMarket->pushOrder(l_order);
}
void Agent::notifyExecution(int a_orderIdentifier,double a_time,int a_price)
{
	Order *l_order = &m_passedOrders[a_orderIdentifier];
	l_order->m_state = EXECUTED;

	// Remove from pending orders list
	concurrency::concurrent_unordered_map<int,Order>::iterator iter;
	mtx_.lock();
	try{
		iter = m_pendingOrders.find(a_orderIdentifier) ;
		if(iter==m_pendingOrders.end()){
			std::cout << "a_orderIdentifier not found in Agent::notifyExecution." << std::endl ;
			exit(1) ;
		}
	}catch(int e)
	{
		std::cout << "notifyExecution find" << e << std::endl;
		exit(1) ;
	}
	try{
		m_pendingOrders.unsafe_erase(iter) ;
	}catch(int e)
	{
		std::cout << "notifyExecution erase" << e << std::endl;
		exit(1) ;
	}
	mtx_.unlock();

	l_order->m_executionHistory.push_back(ExecutionHistory(a_time,l_order->m_volume,a_price));
	updateStockNumber(l_order,l_order->m_volume);
	processCashPosition(a_time,l_order->m_volume,a_price,l_order->m_type);
	l_order->m_volume = 0;
}
void Agent::notifyPartialExecution(int a_orderIdentifier,double a_time,int a_volume,int a_price)
{
	Order *l_order = &m_passedOrders[a_orderIdentifier];
	l_order->m_state = PARTIALLY_EXECUTED;

	l_order->m_executionHistory.push_back(ExecutionHistory(a_time,a_volume,a_price));
	l_order->m_volume -= a_volume;

	updateStockNumber(l_order,a_volume);
	processCashPosition(a_time,a_volume,a_price,l_order->m_type);

}
void Agent::notifyCancellation(int a_orderIdentifier,double a_time)
{
	Order *l_order = &m_passedOrders[a_orderIdentifier];
	l_order->m_state = CANCELED;

	// Remove from pending orders list
	mtx_.lock();
	concurrency::concurrent_unordered_map<int,Order>::iterator iter = m_pendingOrders.find(a_orderIdentifier) ;
	if(iter==m_pendingOrders.end()){
		std::cout << "a_orderIdentifier not found in Agent::notifyCancellation." << std::endl ;
		exit(1) ;
	}
	try{
		m_pendingOrders.unsafe_erase(iter) ;
	}catch(int e)
	{
		std::cout << "notifyExecution erase" << e << std::endl;
		exit(1) ;
	}

	mtx_.unlock();

	l_order->m_executionHistory.push_back(ExecutionHistory(a_time,-1,-1));
}
void Agent::processCashPosition(double a_time,int a_volume,int a_price,const OrderType &a_type)
{
	int l_cashFlow = a_volume*a_price;
	int l_type;
	switch (a_type)
	{
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

	m_cashPosition += (l_type*l_cashFlow);
	m_cashPositionHistory.push_back(std::make_pair(a_time,m_cashPosition));
}
void Agent::notifyAgentOfMarketEvent()
{
	processInformation();
}
void Agent::initAssetQuantity(int a_stockIdentifier)
{
	m_stockQuantity[a_stockIdentifier] = 0;
}	
int Agent::getStockQuantity(int a_stockIdentifier)
{
	return m_stockQuantity[a_stockIdentifier];
}
void Agent::updateStockNumber(Order *a_order,int a_executedVolume)
{
	switch(a_order->m_type)
	{
	case LIMIT_SELL:
	case MARKET_SELL:
		m_stockQuantity[a_order->m_asset] -= a_executedVolume;
		break;
	case LIMIT_BUY:
	case MARKET_BUY:
		m_stockQuantity[a_order->m_asset] += a_executedVolume;
		break;
	default:// Nothing to do with other order types
		break;
	}
}
bool Agent::isMine(Order a_order)const
{
	bool l_mine = false;
	if (a_order.m_owner == m_identifier)
	{
		l_mine = true;
	}
	return l_mine;
}
void Agent::chargeMarketFees(double a_fee)
{
	m_fees += a_fee;
}
double Agent::getNetCashPosition()
{
	return (m_cashPosition-m_fees);
}
int Agent::getTargetedStock()
{
	return getFavouriteStock();
}
double Agent::getFeeRate(OrderType a_orderType)
{
	return 0.0;
}
concurrency::concurrent_unordered_map<int,Order> * Agent::getPendingOrders()
{
	return &m_pendingOrders;
}

 int Agent::nbPendingOrder()
{
	return m_pendingOrders.size();
}