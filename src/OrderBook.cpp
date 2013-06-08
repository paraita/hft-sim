#include "Exceptions.h"
#include "Types.h"
#include "Market.h"
#include "OrderBook.h"
#include "Order.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <boost/foreach.hpp>

OrderBook::OrderBook(Market *a_market, int a_identifier, int a_tickSize, int a_defaultBid, int a_defaultAsk)
{
	m_identifier = a_identifier;
	m_linkToMarket	= a_market;
	m_tickSize = a_tickSize;
	m_defaultBid = a_defaultBid;
	m_defaultAsk = a_defaultAsk;
	m_last = 0;
	m_lastQ= 0;
	m_storeOrderBookHistory = false;
	m_storeOrderHistory = false;
	m_printHistoryonTheFly = false;
	m_maxDepth = 4;
	m_headerPrinted = false;
	m_returnsSumSquared = 0.0;
	m_historicPrices.push_back(10000); 
	m_transactionsTimes.push_back(0);
	totalAskQuantity = 0;
	totalBidQuantity = 0;
	m_activateHFTPriority = false;
	bid = a_defaultBid;
	ask = a_defaultAsk;
	nbOrder = 0;
	quantityExchanged = 0;
	nbOrderMM = 0;
}
OrderBook::~OrderBook()
{

}
void OrderBook::activateHFTPriority(bool activate){
	m_activateHFTPriority = activate;
}
Market * OrderBook::getLinkToMarket()
{
	return m_linkToMarket ;
}
void OrderBook::setStoreOrderBookHistory(bool a_store, int a_depth)
{
	m_storeOrderBookHistory = a_store;
	m_maxDepth = a_depth;
}
void OrderBook::setPrintOrderBookHistory(bool a_print, int a_depth)
{
	m_printHistoryonTheFly = a_print;
	m_maxDepth = a_depth;
}
void OrderBook::setStoreOrderHistory(bool a_store)
{
	m_storeOrderHistory = a_store;
}

int OrderBook::getIdentifier() const
{
	return m_identifier ;
}
int OrderBook::getAskPrice() 
{
	/*if (m_asks.empty())
	return m_defaultAsk;
	else
	return m_asks.begin()->first;*/

	return ask;
}
int OrderBook::getBidPrice() 
{
	/*if (m_bids.empty())
	return m_defaultBid;
	else
	return m_bids.rbegin()->first;*/

	return bid;
}
void OrderBook::updateAskPrice() 
{
	if (m_asks.empty())
		ask = m_defaultAsk;
	else
		ask = m_asks.begin()->first;
}
void OrderBook::updateBidPrice()
{
	if (m_bids.empty())
		bid = m_defaultBid;
	else
		bid = m_bids.rbegin()->first;
}

int OrderBook::getTickSize() const
{
	return m_tickSize;
}
int OrderBook::getBidQuantity()
{
	return quantity.at(getBidPrice());
}
int OrderBook::getAskQuantity()
{
	return quantity.at(getAskPrice());
}

int OrderBook::getNbOrder(){
	return nbOrder;
}

int OrderBook::getNbOrderMM(){
	return nbOrderMM;
}

double OrderBook::getQuantityExchanged(){
	return quantityExchanged;
}

int OrderBook::getDistanceToBestOppositeQuote(int a_price) 
{
	int l_ask = getAskPrice();
	int l_bid = getBidPrice();
	int distance;
	if	(a_price>=l_ask)
	{
		distance = a_price - l_bid;
	}
	else
	{
		distance = l_ask - a_price;
	}
	return (distance/m_tickSize);
}
void OrderBook::runOrderBook()
{
	Order orderToExecute;
	while(open){
		if(orders.try_pop(orderToExecute)){
			switch(orderToExecute.m_type)
			{
			case LIMIT_SELL:
				//std::cout << "limit sell :" << std::endl;
				processLimitSellOrder(orderToExecute);
				break;
			case LIMIT_BUY:
				//std::cout << "limit buy : " << std::endl;
				processLimitBuyOrder(orderToExecute);
				break;
			case MARKET_SELL:
				//std::cout << "market sell" << std::endl;
				processMarketSellOrder(orderToExecute);
				break;
			case MARKET_BUY:
				//std::cout << "market buy" << std::endl;
				processMarketBuyOrder(orderToExecute);
				break;
			case CANCEL_BUY:
				//std::cout << "cancel buy" << std::endl;
				processBuyCancellation(orderToExecute.m_owner, orderToExecute.m_globalOrderIdentifier, orderToExecute.getPrice(), orderToExecute.m_time);
				break;
			case CANCEL_SELL:
				//std::cout << "cancel sell" << std::endl;
				processSellCancellation(orderToExecute.m_owner, orderToExecute.m_globalOrderIdentifier, orderToExecute.getPrice(), orderToExecute.m_time);
				break;
			case CLEAR_OB:
				/*cleanOrderBook();				
				setDefaultBidAsk(orderToExecute.m_newBid, orderToExecute.m_newAsk);*/
				break;
			default:
				break;
			} 
			m_linkToMarket->updateCurrentTime(orderToExecute.m_time);
			m_linkToMarket->notifyAllAgents();
			if(orderToExecute.getOwner() == 3)
				nbOrderMM++;
			nbOrder++;
		}
	}
}
void OrderBook::processLimitBuyOrder(Order & a_order)
{
	// Check if this is not a crossing order
	int ask0 = getAskPrice() ;
	if(ask0<=a_order.m_price)
	{
		processMarketBuyOrder(a_order);
	}
	else
	{
		// If OK, store order
		if(a_order.getOwner() == 3 && m_activateHFTPriority){
			m_bids[a_order.m_price].push_front(a_order);
			updateBidPrice();
		}else{
			m_bids[a_order.m_price].push_back(a_order);
			updateBidPrice();
		}
		quantity[a_order.m_price] = get_value_map( quantity, a_order.m_price, 0 ) + a_order.getVolume();
		int idAgent = a_order.getOwner();
		agentsOrders[idAgent][a_order.m_price] = get_value_map( agentsOrders[idAgent], a_order.m_price, 0 ) + a_order.getVolume();
		totalBidQuantity += a_order.getVolume();
	}
}
void OrderBook::processLimitSellOrder(Order & a_order)
{
	// Check if this is not a crossing order
	int bid0 = getBidPrice() ;
	if(a_order.m_price<=bid0)
	{
		processMarketSellOrder(a_order);
	}
	else
	{
		// If OK, store order
		if(a_order.getOwner() == 3 && m_activateHFTPriority){
			m_asks[a_order.m_price].push_front(a_order);
			updateAskPrice();
		}else{
			m_asks[a_order.m_price].push_back(a_order);
			updateAskPrice();
		}
		quantity[a_order.m_price] = get_value_map( quantity, a_order.m_price, 0 ) + a_order.getVolume();
		int idAgent = a_order.getOwner();
		agentsOrders[idAgent][a_order.m_price] = get_value_map( agentsOrders[idAgent], a_order.m_price, 0 ) + a_order.getVolume();
		totalAskQuantity += a_order.getVolume();
	}
}
void OrderBook::processMarketBuyOrder(Order & a_order)
{
	std::map<int,std::list < Order > > ::iterator iter;
	Order *l_fifoOrder;
	while(a_order.m_volume>0)
	{
		if(!m_asks.empty())
		{
			iter = m_asks.begin();
			l_fifoOrder = &iter->second.front();
			if (l_fifoOrder->m_volume == a_order.m_volume)
			{
				m_linkToMarket->notifyExecution(l_fifoOrder->m_owner,l_fifoOrder->m_globalOrderIdentifier,a_order.m_time,l_fifoOrder->m_price);
				m_linkToMarket->notifyExecution(a_order.m_owner,a_order.m_globalOrderIdentifier,a_order.m_time,l_fifoOrder->m_price);
				a_order.m_volume = 0;
				m_last = l_fifoOrder->m_price;
				m_lastQ = l_fifoOrder->m_volume;
				quantity[l_fifoOrder->m_price] -= l_fifoOrder->m_volume;
				agentsOrders[l_fifoOrder->getOwner()][l_fifoOrder->m_price] -= a_order.getVolume();

				totalAskQuantity -= l_fifoOrder->m_volume;

				quantityExchanged += l_fifoOrder->m_volume;

				iter->second.pop_front();
				updateAskPrice();

			}
			else if (l_fifoOrder->m_volume > a_order.m_volume)
			{

				m_linkToMarket->notifyPartialExecution(l_fifoOrder->m_owner,l_fifoOrder->m_globalOrderIdentifier,a_order.m_time,a_order.m_volume,l_fifoOrder->m_price);
				m_linkToMarket->notifyExecution(a_order.m_owner,a_order.m_globalOrderIdentifier,a_order.m_time,l_fifoOrder->m_price);
				l_fifoOrder->m_volume -= a_order.m_volume;
				quantity[l_fifoOrder->m_price] -= a_order.getVolume();
				agentsOrders[l_fifoOrder->getOwner()][l_fifoOrder->m_price] -= a_order.getVolume();
				totalAskQuantity -= a_order.getVolume();

				quantityExchanged += a_order.getVolume();

				a_order.m_volume = 0;
				m_last = l_fifoOrder->m_price;
				m_lastQ = a_order.m_volume;
			}
			else if (l_fifoOrder->m_volume < a_order.m_volume)
			{

				m_linkToMarket->notifyExecution(l_fifoOrder->m_owner,l_fifoOrder->m_globalOrderIdentifier,a_order.m_time,l_fifoOrder->m_price);
				m_linkToMarket->notifyPartialExecution(a_order.m_owner,a_order.m_globalOrderIdentifier,a_order.m_time,l_fifoOrder->m_volume,l_fifoOrder->m_price);
				a_order.m_volume -= l_fifoOrder->m_volume;
				quantity[l_fifoOrder->m_price] -= l_fifoOrder->m_volume;
				agentsOrders[l_fifoOrder->getOwner()][l_fifoOrder->m_price] -= l_fifoOrder->getVolume();
				totalAskQuantity -= l_fifoOrder->m_volume;

				quantityExchanged += l_fifoOrder->m_volume;

				m_last = l_fifoOrder->m_price;
				m_lastQ = l_fifoOrder->m_volume;

				iter->second.pop_front();
				updateAskPrice();


			}
			if(iter->second.empty() && iter != m_asks.end()){

				m_asks.erase(m_asks.begin()) ;
			}
			m_historicPrices.push_back(m_last);
			m_transactionsTimes.push_back(m_linkToMarket->getCurrentTime());
			int sizePrices = m_historicPrices.size();
			double returns = double(double(double(m_historicPrices[sizePrices-1]) - double(m_historicPrices[sizePrices-2]))/double(m_historicPrices[sizePrices-2]));
			m_returnsSumSquared+=pow(returns,2);
		}
		else
		{
			std::ostringstream l_stream;
			l_stream<<"Not enough ask orders for asset "<<m_identifier;
			std::string l_string = l_stream.str();
			throw Exception(l_string.c_str());
		}
	}
}
void OrderBook::processMarketSellOrder(Order & a_order)
{
	std::map<int,std::list < Order > > ::reverse_iterator iter;
	Order *l_fifoOrder;
	while(a_order.m_volume>0)
	{
		if (!m_bids.empty())
		{
			iter = m_bids.rbegin();
			l_fifoOrder = &iter->second.front();
			if (l_fifoOrder->m_volume == a_order.m_volume)
			{
				m_linkToMarket->notifyExecution(l_fifoOrder->m_owner,l_fifoOrder->m_globalOrderIdentifier,a_order.m_time,l_fifoOrder->m_price);
				m_linkToMarket->notifyExecution(a_order.m_owner,a_order.m_globalOrderIdentifier,a_order.m_time,l_fifoOrder->m_price);
				a_order.m_volume = 0;
				m_last = l_fifoOrder->m_price;
				m_lastQ = l_fifoOrder->m_volume;
				quantity[l_fifoOrder->m_price] -= l_fifoOrder->m_volume;
				agentsOrders[l_fifoOrder->getOwner()][l_fifoOrder->m_price] -= l_fifoOrder->m_volume;
				totalBidQuantity -= l_fifoOrder->m_volume;

				quantityExchanged += l_fifoOrder->m_volume;

				iter->second.pop_front();

				updateBidPrice();

			}
			else if (l_fifoOrder->m_volume > a_order.m_volume)
			{

				m_linkToMarket->notifyPartialExecution(l_fifoOrder->m_owner,l_fifoOrder->m_globalOrderIdentifier,a_order.m_time,a_order.m_volume,l_fifoOrder->m_price);
				m_linkToMarket->notifyExecution(a_order.m_owner,a_order.m_globalOrderIdentifier,a_order.m_time,l_fifoOrder->m_price);
				l_fifoOrder->m_volume -= a_order.m_volume;
				quantity[l_fifoOrder->m_price] -= a_order.getVolume();
				agentsOrders[l_fifoOrder->getOwner()][l_fifoOrder->m_price] -= a_order.getVolume();
				totalBidQuantity -= a_order.getVolume();
				quantityExchanged += a_order.getVolume();
				a_order.m_volume = 0;
				m_last = l_fifoOrder->m_price;
				m_lastQ = l_fifoOrder->m_volume;				
			}
			else if (l_fifoOrder->m_volume < a_order.m_volume)
			{
				m_linkToMarket->notifyExecution(l_fifoOrder->m_owner,l_fifoOrder->m_globalOrderIdentifier,a_order.m_time,l_fifoOrder->m_price);
				m_linkToMarket->notifyPartialExecution(a_order.m_owner,a_order.m_globalOrderIdentifier,a_order.m_time,l_fifoOrder->m_volume,l_fifoOrder->m_price);
				a_order.m_volume -= l_fifoOrder->m_volume;
				quantity[l_fifoOrder->m_price] -= l_fifoOrder->m_volume;
				agentsOrders[l_fifoOrder->getOwner()][l_fifoOrder->m_price] -= l_fifoOrder->m_volume;
				totalBidQuantity -= l_fifoOrder->m_volume;

				quantityExchanged += l_fifoOrder->m_volume;

				m_last = l_fifoOrder->m_price;
				m_lastQ = l_fifoOrder->m_volume;

				iter->second.pop_front();
				updateBidPrice();
			}
			if(iter->second.empty() && iter != m_bids.rend()){
				m_bids.erase( --m_bids.end() ) ;
			}

			m_historicPrices.push_back(m_last);
			m_transactionsTimes.push_back(m_linkToMarket->getCurrentTime());
			int sizePrices = m_historicPrices.size();
			double returns = double(double(double(m_historicPrices[sizePrices-1]) - double(m_historicPrices[sizePrices-2]))/double(m_historicPrices[sizePrices-2]));
			m_returnsSumSquared+=pow(returns,2);

		}
		else
		{
			std::ostringstream l_stream;
			l_stream<<"Not enough ask orders for asset "<<m_identifier;
			std::string l_string = l_stream.str();
			throw Exception(l_string.c_str());
		}
	}
}
int OrderBook::getQuantityForThisPrice(int a_priceLevel)
{
	return get_value_map( quantity, a_priceLevel, 0 );
}

void OrderBook::processSellCancellation(int a_agentIdentifier,int a_orderIdentifier, int a_price, double a_time)
{

	bool orderFoundinAsks = false ;
	std::map<int,std::list<Order> >::iterator itPrice ;
	std::list<Order>::iterator it_l_order ;
	// Look for order in m_asks
	itPrice = m_asks.find(a_price);

	while(!orderFoundinAsks && itPrice != m_asks.end())
	{
		it_l_order = itPrice->second.begin() ;
		while (!orderFoundinAsks && it_l_order!=(*itPrice).second.end())
		{
			if(it_l_order->getIdentifier()==a_orderIdentifier){
				orderFoundinAsks = true ;
				break;
			}
			it_l_order++ ;
		}
		if(orderFoundinAsks) break ;
		itPrice++ ;
	}
	if(orderFoundinAsks)
	{
		// check owner
		if(it_l_order->getOwner()!=a_agentIdentifier){
			std::cout << "Owner mismatch in OrderBook::processCancellation." << std::endl ;
			exit(1) ;
		}

		// erase it
		quantity[it_l_order->m_price] -= it_l_order->m_volume;
		agentsOrders[a_agentIdentifier][it_l_order->m_price] -= it_l_order->m_volume;
		totalAskQuantity -= it_l_order->m_volume;
		itPrice->second.erase(it_l_order) ;

		//pop the queue from the list if empty
		if(itPrice->second.empty())
		{
			m_asks.erase(itPrice);
		}

		updateAskPrice();
		// notify owner of order
		m_linkToMarket->notifyCancellation(a_agentIdentifier,a_orderIdentifier,a_time) ;
		m_linkToMarket->updateCurrentTime(a_time);
		m_linkToMarket->notifyAllAgents();
	}
	// ... else raise error
	else{
		//notify agent
	}
}

void OrderBook::processBuyCancellation(int a_agentIdentifier,int a_orderIdentifier,  int a_price, double a_time)
{
	bool orderFoundinBids = false ;
	std::map<int,std::list<Order> >::iterator itPrice ;
	std::list<Order>::iterator it_l_order ;
	// Look for order in m_asks
	itPrice = m_bids.find(a_price);

	while(!orderFoundinBids && itPrice != m_bids.end())
	{
		it_l_order = itPrice->second.begin() ;
		while (!orderFoundinBids && it_l_order!=(*itPrice).second.end())
		{
			if(it_l_order->getIdentifier()==a_orderIdentifier){
				orderFoundinBids = true ;
				break;
			}
			it_l_order++ ;
		}
		if(orderFoundinBids) break ;
		itPrice++ ;
	}
	if(orderFoundinBids)
	{
		// check owner
		if(it_l_order->getOwner()!=a_agentIdentifier){
			std::cout << "Owner mismatch in OrderBook::processCancellation." << std::endl ;
			exit(1) ;
		}
		quantity[it_l_order->m_price] -= it_l_order->m_volume;
		agentsOrders[a_agentIdentifier][it_l_order->m_price] -= it_l_order->m_volume;
		totalBidQuantity -= it_l_order->m_volume;
		// erase it
		itPrice->second.erase(it_l_order) ;
		//pop the queue from the list if empty
		if(itPrice->second.empty())
		{
			m_bids.erase(itPrice);
		}

		updateBidPrice();
		// notify owner of order
		m_linkToMarket->notifyCancellation(a_agentIdentifier,a_orderIdentifier,a_time) ;
		m_linkToMarket->updateCurrentTime(a_time);
		m_linkToMarket->notifyAllAgents();

	}
	// ... else raise error
	else{
		//notify agent
	}
}

const std::vector<OrderBookHistory> & OrderBook::getOrderBookHistory() const
{
	return m_orderBookHistory;
}

const std::vector<Order> & OrderBook::getOrderHistory() const
{
	return m_orderHistory;
}

int OrderBook::getTotalBidQuantity()
{
	return totalBidQuantity ;
}

int OrderBook::getTotalAskQuantity()
{
	return totalAskQuantity ;
}

void OrderBook::cleanOrderBook(){

	m_asks.clear();
	m_bids.clear();

	quantity.clear();
}

void OrderBook::setDefaultBidAsk(int bid, int ask){
	m_defaultBid = bid;
	m_defaultAsk = ask;
}

concurrency::concurrent_vector<int> OrderBook::getHistoricPrices(){
	return m_historicPrices;
}

concurrency::concurrent_vector<double> OrderBook::getTransactionsTimes(){
	return m_transactionsTimes;
}

double OrderBook::getReturnsSumSquared(){
	return m_returnsSumSquared;
}

void OrderBook::pushOrder(Order &order){
	orders.push(order);
}

void OrderBook::closeOrderBook(){
	open = false;
}

int OrderBook::getPrice() const
{
	return m_last;
}

void OrderBook::getOrderBookForPlot(std::vector<int> &a_price, std::vector<int> &a_priceQ, std::vector<int> &a_priceMM, std::vector<int> &a_priceQMM)
{
	concurrency::concurrent_unordered_map<int, int>::const_iterator itBids; 
	concurrency::concurrent_unordered_map<int, int>::const_iterator itMM; 
	concurrency::concurrent_unordered_map<int, int>::const_iterator it; 
	std::vector<LimitOrders> l_vector;

	for(itBids = quantity.begin();itBids != quantity.end();itBids++)
	{
		//std::cout << "p: " << (*itBids).first << "q: " << (*itBids).second << std::endl;
		if((*itBids).second !=0){
			a_price.push_back((*itBids).first);

			if((*itBids).first <=  getBidPrice())
				a_priceQ.push_back(-1*(*itBids).second);
			else
				a_priceQ.push_back((*itBids).second);
		}
	}

	for(itMM = agentsOrders[3].begin(); itMM != agentsOrders[3].end();itMM++)
	{
		if((*itMM).second !=0){
			a_priceMM.push_back((*itMM).first);
			if((*itMM).first <=  getBidPrice()){
				a_priceQMM.push_back(-1*(*itMM).second);
			}
			else{
				a_priceQMM.push_back((*itMM).second);
			}
		}
	}
	quickSort(a_price, a_priceQ, 0, a_price.size()-1);
	if (a_priceQMM.size() != 0) {
		quickSort(a_priceMM, a_priceQMM, 0, a_priceMM.size()-1);
	}
	/*for(int i = 0; i < a_priceMM.size();i++){
	std::cout<<"sorted       p: " << a_priceMM[i] << " q: " << a_priceQMM[i] << std::endl;
	}*/
}

void OrderBook::quickSort(std::vector<int> &price, std::vector<int> &quantity, int left, int right)  
{  
	int i=left, j=right;  
	int pivot = price[(i+j)/2];  

	// partition  
	while (i <= j) {  
		while (price[i] < pivot)  
			i++;  

		while (price[j] > pivot)  
			j--;  

		if (i <= j) {  
			int tmp = price[i];  
			price[i] = price[j];  
			price[j] = tmp;

			int tmpQ = quantity[i];  
			quantity[i] = quantity[j];  
			quantity[j] = tmpQ;

			i++;  
			j--;  
		}  
	}  

	// recursion  
	if (left < j)  
		quickSort(price, quantity, left, j);  

	if (i < right)  
		quickSort(price, quantity, i, right);  
}  