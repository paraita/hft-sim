#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include "Exceptions.h"
#include "Types.h"
#include "Market.h"
#include "OrderBook.h"
#include "Order.h"

OrderBook::OrderBook(Market *a_market,
		int a_identifier,
		int a_tickSize,
		int a_defaultBid,
		int a_defaultAsk) {
	identifier = a_identifier;
	linkToMarket	= a_market;
	tickSize = a_tickSize;
	defaultBid = a_defaultBid;
	defaultAsk = a_defaultAsk;
	last = 0;
	lastQ= 0;
	b_storeOrderBookHistory = false;
	storeOrderHistory = false;
	printHistoryonTheFly = false;
	maxDepth = 4;
	headerPrinted = false;
	returnsSumSquared = 0.0;
	historicPrices.push_back(10000);
	transactionsTimes.push_back(0);
}

OrderBook::~OrderBook() { }

Market* OrderBook::getLinkToMarket() {
	return linkToMarket;
}

void OrderBook::setStoreOrderBookHistory(bool a_store, int a_depth) {
	b_storeOrderBookHistory = a_store;
	maxDepth = a_depth;
}

void OrderBook::setPrintOrderBookHistory(bool a_print, int a_depth) {
	printHistoryonTheFly = a_print;
	maxDepth = a_depth;
}

void OrderBook::setStoreOrderHistory(bool a_store) {
	storeOrderHistory = a_store;
}

int OrderBook::getIdentifier() const {
	return identifier;
}

int OrderBook::getAskPrice() const {
	if (asks.empty()) return defaultAsk;
	else return asks.begin()->first;
}

int OrderBook::getBidPrice() const {
	if (bids.empty()) return defaultBid;
	else return bids.rbegin()->first;
}

int OrderBook::getTickSize() const {
	return tickSize;
}

int OrderBook::getBidQuantity() const {
	int l_quantity,l_bid;
	std::map<int, std::list<Order> >::const_reverse_iterator it = bids.rbegin();
	l_bid = (*it).first;
	l_quantity = getQuantityForThisPrice(bids,l_bid);
	return l_quantity;
}

int OrderBook::getAskQuantity() const {
	int l_quantity,l_ask;
	std::map<int, std::list<Order> >::const_iterator it = asks.begin();
	l_ask = (*it).first;
	l_quantity = getQuantityForThisPrice(asks,l_ask);
	return l_quantity;
}

int OrderBook::getDistanceToBestOppositeQuote(int a_price) const {
	int l_ask = getAskPrice();
	int l_bid = getBidPrice();
	int distance;
	if(a_price >= l_ask) {
		distance = a_price - l_bid;
	}
	else {
		distance = l_ask - a_price;
	}
	return (distance / tickSize);
}

void OrderBook::processOrder(Order& a_order) {
	switch(a_order.getType()) {
	case LIMIT_SELL:
		processLimitSellOrder(a_order);
		break;
	case LIMIT_BUY:
		processLimitBuyOrder(a_order);
		break;
	case MARKET_SELL:
		processMarketSellOrder(a_order);
		break;
	case MARKET_BUY:
		processMarketBuyOrder(a_order);
		break;
	default:
		break;
	}
	if(storeOrderHistory) orderHistory.push_back(a_order);
	linkToMarket->updateCurrentTime(a_order.getTime());
	linkToMarket->notifyAllAgents();
}

void OrderBook::processLimitBuyOrder(Order& a_order) {
	/* Check if this is not a crossing order */
	int ask0 = getAskPrice();
	if(ask0 <= a_order.getPrice()) {
		std::cout << "Attempting to submit crossing limit order in OrderBook::processLimitBuyOrder." << std::endl;
	}
	/* If OK, store order */
	bids[a_order.getPrice()].push_back(a_order);
	/* If this order is above bid[MAX_LEVEL], then it needs to be recorded in history */
	if(b_storeOrderBookHistory) {
		int bidMAX = getBidPriceAtLevel(maxDepth);
		if(bidMAX<=a_order.getPrice()) {
			storeOrderBookHistory(a_order.getTime());
		}
	}
}

void OrderBook::processLimitSellOrder(Order& a_order) {
	/* Check if this is not a crossing order */
	int bid0 = getBidPrice();
	if(a_order.getPrice() <= bid0) {
		std::cout << "Attempting to submit crossing limit order in OrderBook::processLimitSellOrder." << std::endl;
	}
	/* If OK, store order */
	asks[a_order.getPrice()].push_back(a_order);
	// If this order is above bid[MAX_LEVEL], then it needs to be recorded in history
	if(b_storeOrderBookHistory) {
		int askMAX = getAskPriceAtLevel(maxDepth);
		if(a_order.getPrice() <= askMAX) {
			storeOrderBookHistory(a_order.getTime());
		}
	}
}

void OrderBook::processMarketBuyOrder(Order& a_order) {
	std::map<int, std::list<Order> >::iterator iter;
	while(a_order.getVolume() > 0) {
		if(!asks.empty()) {
			iter = asks.begin();
			Order *l_fifoOrder = &iter->second.front();
			if (l_fifoOrder->getVolume() == a_order.getVolume()) {
				linkToMarket->notifyExecution(l_fifoOrder->getOwner(),
						l_fifoOrder->getGlobalOrderIdentifier(),
						a_order.getTime(),
						l_fifoOrder->getPrice());
				linkToMarket->notifyExecution(a_order.getOwner(),
						a_order.getGlobalOrderIdentifier(),
						a_order.getTime(),
						l_fifoOrder->getPrice());
				a_order.setVolume(0);
				last = l_fifoOrder->getPrice();
				lastQ = l_fifoOrder->getVolume();
				if (b_storeOrderBookHistory) {
					storeOrderBookHistory(a_order.getTime());
				}
				if (printHistoryonTheFly) {
					printOrderBookHistoryOnTheFly(a_order.getTime());
				}
				iter->second.pop_front();
			}
			else if (l_fifoOrder->getVolume() > a_order.getVolume()) {
				linkToMarket->notifyPartialExecution(l_fifoOrder->getOwner(),
						l_fifoOrder->getGlobalOrderIdentifier(),
						a_order.getTime(),
						a_order.getVolume(),
						l_fifoOrder->getPrice());
				linkToMarket->notifyExecution(a_order.getOwner(),
						a_order.getGlobalOrderIdentifier(),
						a_order.getTime(),
						l_fifoOrder->getPrice());
				l_fifoOrder->setVolume(l_fifoOrder->getVolume() - a_order.getVolume());
				a_order.setVolume(0);
				last = l_fifoOrder->getPrice();
				lastQ = l_fifoOrder->getVolume();
				if (b_storeOrderBookHistory) {
					storeOrderBookHistory(a_order.getTime());
				}
				if (printHistoryonTheFly) {
					printOrderBookHistoryOnTheFly(a_order.getTime());
				}
			}
			else if (l_fifoOrder->getVolume() < a_order.getVolume()) {
				linkToMarket->notifyExecution(l_fifoOrder->getOwner(),
						l_fifoOrder->getGlobalOrderIdentifier(),
						a_order.getTime(),
						l_fifoOrder->getPrice());
				linkToMarket->notifyPartialExecution(a_order.getOwner(),
						a_order.getGlobalOrderIdentifier(),
						a_order.getTime(),
						l_fifoOrder->getVolume(),
						l_fifoOrder->getPrice());
				a_order.setVolume(a_order.getVolume() - l_fifoOrder->getVolume());
				last = l_fifoOrder->getPrice();
				lastQ = l_fifoOrder->getVolume();
				if (b_storeOrderBookHistory) {
					storeOrderBookHistory(a_order.getTime());
				}
				if (printHistoryonTheFly) {
					printOrderBookHistoryOnTheFly(a_order.getTime());
				}
				iter->second.pop_front();
			}
			if(iter->second.empty() && iter != asks.end()) {
				asks.erase(asks.begin());
			}
			historicPrices.push_back(last);
			transactionsTimes.push_back(linkToMarket->getCurrentTime());
			int sizePrices = historicPrices.size();
			double returns = double(double(double(historicPrices[sizePrices-1]) - double(historicPrices[sizePrices-2]))/double(historicPrices[sizePrices-2]));
			returnsSumSquared+=pow(returns, 2);
		}
		else {
			std::ostringstream l_stream;
			l_stream << "Not enough ask orders for asset " << identifier;
			std::string l_string = l_stream.str();
			throw Exception(l_string.c_str());
		}
	}
}

void OrderBook::processMarketSellOrder(Order& a_order) {
	std::map<int, std::list<Order> >::reverse_iterator iter;
	while(a_order.getVolume() > 0) {
		if (!bids.empty()) {
			iter = bids.rbegin();
			Order *l_fifoOrder = &iter->second.front();
			if (l_fifoOrder->getVolume() == a_order.getVolume()) {
				linkToMarket->notifyExecution(l_fifoOrder->getOwner(),
						l_fifoOrder->getGlobalOrderIdentifier(),
						a_order.getTime(),
						l_fifoOrder->getPrice());
				linkToMarket->notifyExecution(a_order.getOwner(),
						a_order.getGlobalOrderIdentifier(),
						a_order.getTime(),
						l_fifoOrder->getPrice());
				a_order.setVolume(0);
				last = l_fifoOrder->getPrice();
				lastQ = l_fifoOrder->getVolume();
				if (b_storeOrderBookHistory) {
					storeOrderBookHistory(a_order.getTime());
				}
				if (printHistoryonTheFly) {
					printOrderBookHistoryOnTheFly(a_order.getTime());
				}
				iter->second.pop_front();
			}
			else if (l_fifoOrder->getVolume() > a_order.getVolume()) {
				linkToMarket->notifyPartialExecution(l_fifoOrder->getOwner(),
						l_fifoOrder->getGlobalOrderIdentifier(),
						a_order.getTime(),
						a_order.getVolume(),
						l_fifoOrder->getPrice());
				linkToMarket->notifyExecution(a_order.getOwner(),
						a_order.getGlobalOrderIdentifier(),
						a_order.getTime(),
						l_fifoOrder->getPrice());
				l_fifoOrder->setVolume(l_fifoOrder->getVolume() - a_order.getVolume());
				a_order.setVolume(0);
				last = l_fifoOrder->getPrice();
				lastQ = l_fifoOrder->getVolume();
				if (b_storeOrderBookHistory) {
					storeOrderBookHistory(a_order.getTime());
				}
				if (printHistoryonTheFly) {
					printOrderBookHistoryOnTheFly(a_order.getTime());
				}
			}
			else if (l_fifoOrder->getVolume() < a_order.getVolume()) {
				linkToMarket->notifyExecution(l_fifoOrder->getOwner(),
						l_fifoOrder->getGlobalOrderIdentifier(),
						a_order.getTime(),
						l_fifoOrder->getPrice());
				linkToMarket->notifyPartialExecution(a_order.getOwner(),
						a_order.getGlobalOrderIdentifier(),
						a_order.getTime(),
						l_fifoOrder->getVolume(),
						l_fifoOrder->getPrice());
				a_order.setVolume(a_order.getVolume() - l_fifoOrder->getVolume());
				last = l_fifoOrder->getPrice();
				lastQ = l_fifoOrder->getVolume();
				if (b_storeOrderBookHistory) {
					storeOrderBookHistory(a_order.getTime());
				}
				if (printHistoryonTheFly) {
					printOrderBookHistoryOnTheFly(a_order.getTime());
				}
				iter->second.pop_front();
			}
			if(iter->second.empty() && iter != bids.rend()) {
				bids.erase(--bids.end());
			}

			historicPrices.push_back(last);

			transactionsTimes.push_back(linkToMarket->getCurrentTime());
			int sizePrices = historicPrices.size();
			double returns = double(double(double(historicPrices[sizePrices-1]) - double(historicPrices[sizePrices-2]))/double(historicPrices[sizePrices-2]));
			returnsSumSquared+=pow(returns,2);

		}
		else {
			std::ostringstream l_stream;
			l_stream << "Not enough ask orders for asset " << identifier;
			std::string l_string = l_stream.str();
			throw Exception(l_string.c_str());
		}
	}
}

int OrderBook::getQuantityForThisPrice(std::map<int, std::list<Order> >  a_orderMap, int a_priceLevel) const {
	int l_quantity = 0;
	std::list<Order> l_orderQueue = a_orderMap[a_priceLevel];
	std::list<Order>::const_iterator it;
	for(it = l_orderQueue.begin(); it != l_orderQueue.end(); it++) {
		l_quantity += ((Order) (*it)).getVolume();
	}
	return l_quantity;
}

int OrderBook::getBidQuantityUpToPrice(int a_price) const {
	int l_quantity = 0;
	std::map<int, std::list<Order> >::const_reverse_iterator it;
	it = bids.rbegin();
	while ((*it).first >= a_price && it != bids.rend()) {
		l_quantity += getQuantityForThisPrice(bids,(*it).first);
		it++;
	}
	return l_quantity;
}

int OrderBook::getAskQuantityUpToPrice(int a_price) const {
	int l_quantity = 0;
	std::map<int, std::list<Order> >::const_iterator it;
	it = asks.begin();
	while ((*it).first <= a_price && it != asks.end()) {
		l_quantity += getQuantityForThisPrice(asks,(*it).first);
		it++;
	}
	return l_quantity;
}

OrderBookHistory OrderBook::buildAHistoryLine(double a_time) const {
	int MAX_LEVEL = maxDepth;

	std::map<int, std::list<Order> >::const_reverse_iterator itBids;
	std::map<int, std::list<Order> >::const_iterator itAsks;
	itBids = bids.rbegin();
	itAsks = asks.begin();

	std::vector<LimitOrders> l_vector;
	for(int l_count = 0; l_count < MAX_LEVEL; l_count++) {
		/* If no orders at a sufficient depth, their value is put to zero */
		int l_bid = 0;
		int l_ask = 0;
		int l_bidQ = 0;
		int l_askQ = 0;
		if(itBids != bids.rend()) {
			l_bid = (*itBids).first;
			l_bidQ = getQuantityForThisPrice(bids, l_bid);
			itBids++;
		}
		if(itAsks != asks.end()) {
			l_ask = (*itAsks).first;
			l_askQ = getQuantityForThisPrice(asks, l_ask);
			itAsks++;
		}
		LimitOrders l_limitOrders(l_bid, l_bidQ, l_ask, l_askQ);
		l_vector.push_back(l_limitOrders);
	}
	OrderBookHistory l_line(1.0, a_time, l_vector, last, lastQ);
	return l_line;
}

void OrderBook::storeOrderBookHistory(double a_time) {
	OrderBookHistory l_line = buildAHistoryLine(a_time);
	orderBookHistory.push_back(l_line);
}

void OrderBook::printLineOfHistory(OrderBookHistory a_line,std::ofstream& a_outFile) {
	std::vector<LimitOrders> l_limitOrders;

	double l_datexl = a_line.datexl;
	double l_ts = a_line.timestamp;
	double l_last = a_line.last;
	double l_lastQ = a_line.lastQ;
	double l_lastF = 1.0;
	l_limitOrders = a_line.limitOrders;
	a_outFile << l_datexl << ',' << l_ts << ',';
	for(int i = 0; i < maxDepth; i++) {
		LimitOrders lo = l_limitOrders[i];
		a_outFile << (double) lo.bid / 100.0 << ','
				<< (double) lo.bidQ << ','
				<< (double) lo.ask / 100.0 << ','
				<< (double) lo.askQ << ',';
	}
	a_outFile << l_last / 100.0 << ',' << l_lastQ << ',' << l_lastF << std::endl;
}

void OrderBook::printStoredOrderBookHistory() {
	std::ostringstream l_stream;
	l_stream << "./res/" << linkToMarket->getName() << "/Market" << identifier << "_OrderBookHistory.csv";
	std::string fileName = l_stream.str();
	std::ofstream outFile(fileName.c_str());

	printHeader(outFile);

	std::vector<OrderBookHistory>::const_iterator it;
	for(it = orderBookHistory.begin(); it != orderBookHistory.end(); it++) {
		printLineOfHistory(*it,outFile);
	}
	outFile.close();
}

void OrderBook::printOrderBookHistoryOnTheFly(double a_time) {  
	std::ostringstream l_stream;
	l_stream << "csv" << identifier << ".txt";
	std::string fileName = l_stream.str();
	std::ofstream outFile(fileName.c_str());

	if(!headerPrinted) {
		printHeader(outFile);
		headerPrinted = true;
	}
	OrderBookHistory l_line = buildAHistoryLine(a_time);
	printLineOfHistory(l_line,outFile);
	outFile.close();
}

void OrderBook::printHeader(std::ofstream& a_outFile) const {
	a_outFile << "date_xl,ts,";
	for(int i = 0; i < maxDepth; i++) {
		a_outFile << "bid" << i + 1 << ",bidQ"
				<< i + 1 << ",ask" << i + 1 << ",askQ" << i + 1;
	}
	a_outFile << ",last,lastQ,lastF" << std::endl;
}

int OrderBook::getPrice() const {
	return last;
}

void OrderBook::getOrderBookForPlot(std::vector<int>& a_price,
		std::vector<int>& a_priceQ,
		std::vector<int>& MMprices,
		std::vector<int>& MMvolumes) const {

	std::map<int, std::list<Order> >::const_iterator itBids;
	std::map<int, std::list<Order> >::const_iterator itAsks;

	std::vector<LimitOrders> l_vector;

	for(itBids = bids.begin(); itBids != bids.end(); itBids++) {
		int l_bid = (*itBids).first;

		std::list<Order> l_order_bid = (*itBids).second;
		int volume_mm_bid = 0;

		for(std::list<Order>::iterator it = l_order_bid.begin(); it !=l_order_bid.end(); it++) {
			/* std::cout<<"Agent ID : "<<it->getOwner()<<std::endl; */
			if(it->getOwner() == 2) {
				volume_mm_bid+=it->getVolume();
			}
		}

		if (volume_mm_bid != 0) {
			MMprices.push_back(l_bid);
			MMvolumes.push_back(-1 * volume_mm_bid);
		}
		a_price.push_back(l_bid);

		a_priceQ.push_back(-1 * getQuantityForThisPrice(bids, l_bid));
	}
	for(itAsks = asks.begin(); itAsks != asks.end(); itAsks++) {
		int l_ask = (*itAsks).first;
		std::list<Order> l_order_ask = (*itAsks).second;
		int volume_mm_ask = 0;
		for (std::list<Order>::iterator it = l_order_ask.begin(); it !=l_order_ask.end(); it++) {
			/* std::cout<<"Agent ID : "<<it->getOwner()<<std::endl; */
			if(it->getOwner() == 2) {
				volume_mm_ask+=it->getVolume();
			}
		}

		if (volume_mm_ask != 0) {
			MMprices.push_back(l_ask);
			MMvolumes.push_back(volume_mm_ask);
		}
		a_price.push_back(l_ask);
		a_priceQ.push_back(getQuantityForThisPrice(asks, l_ask));
	}
}

std::map<int, std::list<Order> > OrderBook::getBidQueue() const {
	return bids;
}

std::map<int, std::list<Order> > OrderBook::getAskQueue() const {
	return asks;
}

int OrderBook::getNumberOfOrders(int a_agentIdentifier, int a_price) const {
	int l_bid = getBidPrice();
	int l_ask = getAskPrice();
	int l_number = 0;
	if ((a_price < l_ask) && (a_price > l_bid)) {
		l_number = 0;
	}
	else if (a_price >= l_ask) {
		std::map<int, std::list<Order> >::const_iterator it = asks.find(a_price);
		if(it != asks.end()) {
			std::list<Order>::const_iterator l_order;
			for (l_order=(*it).second.begin(); l_order!=(*it).second.end(); l_order++) {
				if (((Order) (*l_order)).getOwner() == a_agentIdentifier) l_number++;
			}
		}
	}
	else {
		std::map<int, std::list<Order> >::const_iterator it = bids.find(a_price);
		if(it != bids.end()) {
			std::list<Order>::const_iterator l_order;
			for (l_order=(*it).second.begin(); l_order!=(*it).second.end(); l_order++) {
				if (((Order) (*l_order)).getOwner() == a_agentIdentifier) l_number++;
			}
		}
	}
	return l_number;
}

void OrderBook::processCancellation(int a_agentIdentifier,int a_orderIdentifier, double a_time) {
	bool orderFoundinAsks = false;
	bool orderFoundinBids = false;
	std::map<int, std::list<Order> >::iterator itPrice;
	std::list<Order>::iterator it_l_order;
	// Look for order in asks
	itPrice = asks.begin();
	while(!orderFoundinAsks && itPrice != asks.end()) {
		it_l_order = itPrice->second.begin();
		while (!orderFoundinAsks && it_l_order != (*itPrice).second.end()) {
			if(it_l_order->getIdentifier() == a_orderIdentifier) {
				orderFoundinAsks = true;
				break;
			}
			it_l_order++;
		}
		if(orderFoundinAsks) break;
		itPrice++;
	}
	/* If not found, look for order in bids */
	if(!orderFoundinAsks) {
		itPrice = bids.begin();
		while(!orderFoundinBids && itPrice != bids.end()) {
			it_l_order = itPrice->second.begin();
			while (!orderFoundinBids && it_l_order!=(*itPrice).second.end()) {
				if(it_l_order->getIdentifier()==a_orderIdentifier) {
					orderFoundinBids = true;
					break;
				}
				it_l_order++;
			}
			if(orderFoundinBids) break;
			itPrice++;
		}
	}
	if(orderFoundinBids || orderFoundinAsks) {
		/* check owner */
		if(it_l_order->getOwner() != a_agentIdentifier) {
			std::cout << "Owner mismatch in OrderBook::processCancellation." << std::endl;
			exit(1);
		}
		/* erase it */
		itPrice->second.erase(it_l_order);
		/* pop the queue from the list if empty */
		if(itPrice->second.empty()) {
			if(orderFoundinBids) {
				bids.erase(itPrice);
			}
			else
			{
				asks.erase(itPrice);
			}
		}
		/* notify owner of order */
		linkToMarket->notifyCancellation(a_agentIdentifier,a_orderIdentifier,a_time);
		linkToMarket->updateCurrentTime(a_time);
		linkToMarket->notifyAllAgents();
	}
	/* ... else raise error */
	else {
		std::cout << "Order not found in OrderBook::processCancellation." << std::endl;
		exit(1);
	}
}

const std::vector<OrderBookHistory>& OrderBook::getOrderBookHistory() const {
	return orderBookHistory;
}

const std::vector<Order>& OrderBook::getOrderHistory() const {
	return orderHistory;
}

int OrderBook::getTotalBidQuantity() {
	long totalBidQuantity = 0;
	std::map<int, std::list<Order> >::iterator it = bids.begin();
	while(it != bids.end()) {
		std::list<Order>::iterator l_order = (*it).second.begin();
		while(l_order!=(*it).second.end()) {
			totalBidQuantity += (*l_order).getVolume();
			l_order++;
		}
		it++;
	}
	return totalBidQuantity;
}

int OrderBook::getTotalAskQuantity() {
	long totalAskQuantity = 0;
	std::map<int, std::list<Order> >::iterator it = asks.begin();
	while(it != asks.end()) {
		std::list<Order>::iterator l_order = (*it).second.begin();
		while(l_order!=(*it).second.end()) {
			totalAskQuantity += (*l_order).getVolume();
			l_order++;
		}
		it++;
	}
	return totalAskQuantity;
}

int OrderBook::getBidPriceAtLevel(int levelMax)
{
	std::map<int, std::list<Order> >::const_reverse_iterator it = bids.rbegin();
	int levelCount = 0;
	while(it!=bids.rend() && levelCount<levelMax) {
		if(!it->second.empty()) {
			levelCount++;
		}
		it++;
	}
	if(it == bids.rend()) it--;
	return it->first;
}

int OrderBook::getAskPriceAtLevel(int levelMax) {
	std::map<int, std::list<Order> >::const_iterator it = asks.begin();
	int levelCount = 0;
	while(it!=asks.end() && levelCount<levelMax) {
		if(!it->second.empty()) {
			levelCount++;
		}
		it++;
	}
	if(it == asks.end()) it--;
	return it->first;
}

int OrderBook::getTotalVolumeAtPrice(int a_price) const {
	int l_bid = getBidPrice();
	int l_ask = getAskPrice();
	int l_number = 0;
	if ((a_price < l_ask) && (a_price > l_bid)) {
		l_number = 0;
	}
	else if (a_price >= l_ask) {
		std::map<int, std::list<Order> >::const_iterator it = asks.find(a_price);
		if(it != asks.end()) {
			std::list<Order>::const_iterator l_order;
			for (l_order=(*it).second.begin();l_order!=(*it).second.end();l_order++) {
				l_number++;
			}
		}
	}
	else {
		std::map<int,std::list<Order> >::const_iterator it = bids.find(a_price);
		if(it != bids.end()) {
			std::list<Order>::const_iterator l_order;
			for (l_order=(*it).second.begin(); l_order != (*it).second.end(); l_order++) {
				l_number++;
			}
		}
	}
	return l_number;
}

void OrderBook::cleanOrderBook() {
	asks.clear();
	bids.clear();
}

void OrderBook::setDefaultBidAsk(int bid, int ask) {
	defaultBid = bid;
	defaultAsk = ask;
}

std::vector<int> OrderBook::getHistoricPrices() {
	return historicPrices;
}

std::vector<double> OrderBook::getTransactionsTimes() {
	return transactionsTimes;
}

double OrderBook::getReturnsSumSquared() {
	return returnsSumSquared;
}
