#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
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
}

OrderBook::~OrderBook() { }

Market* OrderBook::getLinkToMarket() {
  return m_linkToMarket;
}

void OrderBook::setStoreOrderBookHistory(bool a_store, int a_depth) {
  m_storeOrderBookHistory = a_store;
  m_maxDepth = a_depth;
}

void OrderBook::setPrintOrderBookHistory(bool a_print, int a_depth) {
  m_printHistoryonTheFly = a_print;
  m_maxDepth = a_depth;
}

void OrderBook::setStoreOrderHistory(bool a_store) {
  m_storeOrderHistory = a_store;
}

int OrderBook::getIdentifier() const {
  return m_identifier;
}

int OrderBook::getAskPrice() const {
  if (m_asks.empty()) return m_defaultAsk;
  else return m_asks.begin()->first;
}

int OrderBook::getBidPrice() const {
  if (m_bids.empty()) return m_defaultBid;
  else return m_bids.rbegin()->first;
}

int OrderBook::getTickSize() const {
  return m_tickSize;
}

int OrderBook::getBidQuantity() const {
  int l_quantity,l_bid;
  std::map<int, std::list<Order> >::const_reverse_iterator it = m_bids.rbegin();
  l_bid = (*it).first;
  l_quantity = getQuantityForThisPrice(m_bids,l_bid);
  return l_quantity;
}

int OrderBook::getAskQuantity() const {
  int l_quantity,l_ask;
  std::map<int, std::list<Order> >::const_iterator it = m_asks.begin();
  l_ask = (*it).first;
  l_quantity = getQuantityForThisPrice(m_asks,l_ask);
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
  return (distance / m_tickSize);
}

void OrderBook::processOrder(Order& a_order) {
  switch(a_order.m_type) {
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
  if(m_storeOrderHistory) m_orderHistory.push_back(a_order);
  m_linkToMarket->updateCurrentTime(a_order.m_time);
  m_linkToMarket->notifyAllAgents();
}

void OrderBook::processLimitBuyOrder(Order& a_order) {
  /* Check if this is not a crossing order */
  int ask0 = getAskPrice();
  if(ask0 <= a_order.m_price) {
    std::cout << "Attempting to submit crossing limit order in OrderBook::processLimitBuyOrder." << std::endl;
  }
  /* If OK, store order */
  m_bids[a_order.m_price].push_back(a_order);
  /* If this order is above bid[MAX_LEVEL], then it needs to be recorded in history */
  if(m_storeOrderBookHistory) {
    int bidMAX = getBidPriceAtLevel(m_maxDepth);
    if(bidMAX<=a_order.m_price) {
      storeOrderBookHistory(a_order.m_time);
    }
  }
}

void OrderBook::processLimitSellOrder(Order& a_order) {
  /* Check if this is not a crossing order */
  int bid0 = getBidPrice();
  if(a_order.m_price <= bid0) {
    std::cout << "Attempting to submit crossing limit order in OrderBook::processLimitSellOrder." << std::endl;
  }
  /* If OK, store order */
  m_asks[a_order.m_price].push_back(a_order);
  // If this order is above bid[MAX_LEVEL], then it needs to be recorded in history
  if(m_storeOrderBookHistory) {
    int askMAX = getAskPriceAtLevel(m_maxDepth);
    if(a_order.m_price <= askMAX) {
      storeOrderBookHistory(a_order.m_time);
    }
  }
}

void OrderBook::processMarketBuyOrder(Order& a_order) {
  std::map<int, std::list<Order> >::iterator iter;
  while(a_order.m_volume > 0) {
    if(!m_asks.empty()) {
      iter = m_asks.begin();
      Order *l_fifoOrder = &iter->second.front();
      if (l_fifoOrder->m_volume == a_order.m_volume) {
	m_linkToMarket->notifyExecution(l_fifoOrder->m_owner,
					l_fifoOrder->m_globalOrderIdentifier,
					a_order.m_time,
					l_fifoOrder->m_price);
	m_linkToMarket->notifyExecution(a_order.m_owner,
					a_order.m_globalOrderIdentifier,
					a_order.m_time,
					l_fifoOrder->m_price);
	a_order.m_volume = 0;
	m_last = l_fifoOrder->m_price;
	m_lastQ = l_fifoOrder->m_volume;
	if (m_storeOrderBookHistory) {
	  storeOrderBookHistory(a_order.m_time);
	}
	if (m_printHistoryonTheFly) {
	  printOrderBookHistoryOnTheFly(a_order.m_time);
	}
	iter->second.pop_front();
      }
      else if (l_fifoOrder->m_volume > a_order.m_volume) {
	m_linkToMarket->notifyPartialExecution(l_fifoOrder->m_owner,
					       l_fifoOrder->m_globalOrderIdentifier,
					       a_order.m_time,
					       a_order.m_volume,
					       l_fifoOrder->m_price);
	m_linkToMarket->notifyExecution(a_order.m_owner,
					a_order.m_globalOrderIdentifier,
					a_order.m_time,
					l_fifoOrder->m_price);
	l_fifoOrder->m_volume -= a_order.m_volume;
	a_order.m_volume = 0;
	m_last = l_fifoOrder->m_price;
	m_lastQ = l_fifoOrder->m_volume;
	if (m_storeOrderBookHistory) {
	  storeOrderBookHistory(a_order.m_time);
	}
	if (m_printHistoryonTheFly) {
	  printOrderBookHistoryOnTheFly(a_order.m_time);
	}
      }
      else if (l_fifoOrder->m_volume < a_order.m_volume) {
	m_linkToMarket->notifyExecution(l_fifoOrder->m_owner,
					l_fifoOrder->m_globalOrderIdentifier,
					a_order.m_time,
					l_fifoOrder->m_price);
	m_linkToMarket->notifyPartialExecution(a_order.m_owner,
					       a_order.m_globalOrderIdentifier,
					       a_order.m_time,
					       l_fifoOrder->m_volume,
					       l_fifoOrder->m_price);
	a_order.m_volume -= l_fifoOrder->m_volume;
	m_last = l_fifoOrder->m_price;
	m_lastQ = l_fifoOrder->m_volume;
	if (m_storeOrderBookHistory) {
	  storeOrderBookHistory(a_order.m_time);
	}
	if (m_printHistoryonTheFly) {
	  printOrderBookHistoryOnTheFly(a_order.m_time);
	}
	iter->second.pop_front();
      }
      if(iter->second.empty() && iter != m_asks.end()) {
	m_asks.erase(m_asks.begin());
      }
      m_historicPrices.push_back(m_last);
      m_transactionsTimes.push_back(m_linkToMarket->getCurrentTime());
      int sizePrices = m_historicPrices.size();
      double returns = double(double(double(m_historicPrices[sizePrices-1]) - double(m_historicPrices[sizePrices-2]))/double(m_historicPrices[sizePrices-2]));
      m_returnsSumSquared+=pow(returns, 2);
    }
    else {
      std::ostringstream l_stream;
      l_stream << "Not enough ask orders for asset " << m_identifier;
      std::string l_string = l_stream.str();
      throw Exception(l_string.c_str());
    }
  }
}

void OrderBook::processMarketSellOrder(Order& a_order) {
  std::map<int, std::list<Order> >::reverse_iterator iter;
  while(a_order.m_volume > 0) {
    if (!m_bids.empty()) {
      iter = m_bids.rbegin();
      Order *l_fifoOrder = &iter->second.front();
      if (l_fifoOrder->m_volume == a_order.m_volume) {
	m_linkToMarket->notifyExecution(l_fifoOrder->m_owner,
					l_fifoOrder->m_globalOrderIdentifier,
					a_order.m_time,
					l_fifoOrder->m_price);
	m_linkToMarket->notifyExecution(a_order.m_owner,
					a_order.m_globalOrderIdentifier,
					a_order.m_time,
					l_fifoOrder->m_price);
	a_order.m_volume = 0;
	m_last = l_fifoOrder->m_price;
	m_lastQ = l_fifoOrder->m_volume;
	if (m_storeOrderBookHistory) {
	  storeOrderBookHistory(a_order.m_time);
	}
	if (m_printHistoryonTheFly) {
	  printOrderBookHistoryOnTheFly(a_order.m_time);
	}
	iter->second.pop_front();
      }
      else if (l_fifoOrder->m_volume > a_order.m_volume) {
	m_linkToMarket->notifyPartialExecution(l_fifoOrder->m_owner,
					       l_fifoOrder->m_globalOrderIdentifier,
					       a_order.m_time,
					       a_order.m_volume,
					       l_fifoOrder->m_price);
	m_linkToMarket->notifyExecution(a_order.m_owner,
					a_order.m_globalOrderIdentifier,
					a_order.m_time,
					l_fifoOrder->m_price);
	l_fifoOrder->m_volume -= a_order.m_volume;
	a_order.m_volume = 0;
	m_last = l_fifoOrder->m_price;
	m_lastQ = l_fifoOrder->m_volume;
	if (m_storeOrderBookHistory) {
	  storeOrderBookHistory(a_order.m_time);
	}
	if (m_printHistoryonTheFly) {
	  printOrderBookHistoryOnTheFly(a_order.m_time);
	}
      }
      else if (l_fifoOrder->m_volume < a_order.m_volume) {
	m_linkToMarket->notifyExecution(l_fifoOrder->m_owner,
					l_fifoOrder->m_globalOrderIdentifier,
					a_order.m_time,
					l_fifoOrder->m_price);
	m_linkToMarket->notifyPartialExecution(a_order.m_owner,
					       a_order.m_globalOrderIdentifier,
					       a_order.m_time,
					       l_fifoOrder->m_volume,
					       l_fifoOrder->m_price);
	a_order.m_volume -= l_fifoOrder->m_volume;
	m_last = l_fifoOrder->m_price;
	m_lastQ = l_fifoOrder->m_volume;
	if (m_storeOrderBookHistory) {
	  storeOrderBookHistory(a_order.m_time);
	}
	if (m_printHistoryonTheFly) {
	  printOrderBookHistoryOnTheFly(a_order.m_time);
	}
	iter->second.pop_front();
      }
      if(iter->second.empty() && iter != m_bids.rend()) {
	m_bids.erase(--m_bids.end());
      }
	
      m_historicPrices.push_back(m_last);
	
      m_transactionsTimes.push_back(m_linkToMarket->getCurrentTime());
      int sizePrices = m_historicPrices.size();
      double returns = double(double(double(m_historicPrices[sizePrices-1]) - double(m_historicPrices[sizePrices-2]))/double(m_historicPrices[sizePrices-2]));
      m_returnsSumSquared+=pow(returns,2);
	
    }
    else {
      std::ostringstream l_stream;
      l_stream << "Not enough ask orders for asset " << m_identifier;
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
    l_quantity += (*it).m_volume;
  }
  return l_quantity;
}

int OrderBook::getBidQuantityUpToPrice(int a_price) const {
  int l_quantity = 0;
  std::map<int, std::list<Order> >::const_reverse_iterator it;
  it = m_bids.rbegin();
  while ((*it).first >= a_price && it != m_bids.rend()) {
    l_quantity += getQuantityForThisPrice(m_bids,(*it).first);
    it++;
  }
  return l_quantity;
}

int OrderBook::getAskQuantityUpToPrice(int a_price) const {
  int l_quantity = 0;
  std::map<int, std::list<Order> >::const_iterator it;
  it = m_asks.begin();
  while ((*it).first <= a_price && it != m_asks.end()) {
    l_quantity += getQuantityForThisPrice(m_asks,(*it).first);
    it++;
  }
  return l_quantity;
}

OrderBookHistory OrderBook::buildAHistoryLine(double a_time) const {
  int MAX_LEVEL = m_maxDepth;
  
  std::map<int, std::list<Order> >::const_reverse_iterator itBids; 
  std::map<int, std::list<Order> >::const_iterator itAsks; 
  itBids = m_bids.rbegin();
  itAsks = m_asks.begin();
	
  std::vector<LimitOrders> l_vector;
  for(int l_count = 0; l_count < MAX_LEVEL; l_count++) {
    /* If no orders at a sufficient depth, their value is put to zero */
    int l_bid = 0;
    int l_ask = 0;
    int l_bidQ = 0;
    int l_askQ = 0;
    if(itBids != m_bids.rend()) {
      l_bid = (*itBids).first;
      l_bidQ = getQuantityForThisPrice(m_bids, l_bid);
      itBids++;
    }
    if(itAsks != m_asks.end()) {
      l_ask = (*itAsks).first;
      l_askQ = getQuantityForThisPrice(m_asks, l_ask);
      itAsks++;
    }
    LimitOrders l_limitOrders(l_bid, l_bidQ, l_ask, l_askQ);
    l_vector.push_back(l_limitOrders);
  }
  OrderBookHistory l_line(1.0, a_time, l_vector, m_last, m_lastQ);
  return l_line;
}

void OrderBook::storeOrderBookHistory(double a_time) {
  OrderBookHistory l_line = buildAHistoryLine(a_time);
  m_orderBookHistory.push_back(l_line);
}

void OrderBook::printLineOfHistory(OrderBookHistory a_line,std::ofstream& a_outFile) {
  std::vector<LimitOrders> l_limitOrders;
		
  double l_datexl = a_line.m_datexl;
  double l_ts = a_line.m_timeStamp;
  double l_last = a_line.m_last;
  double l_lastQ = a_line.m_lastQ;
  double l_lastF = 1.0;
  l_limitOrders = a_line.m_limitOrders;
  a_outFile << l_datexl << ',' << l_ts << ',';
  for(int i = 0; i < m_maxDepth; i++) {
    a_outFile << (double) l_limitOrders[i].m_bid / 100.0 << ','
	      << (double) l_limitOrders[i].m_bidQ << ','
	      << (double) l_limitOrders[i].m_ask / 100.0 << ','
	      << (double) l_limitOrders[i].m_askQ << ',';
  }
  a_outFile << l_last / 100.0 << ',' << l_lastQ << ',' << l_lastF << std::endl;
}

void OrderBook::printStoredOrderBookHistory() {
  std::ostringstream l_stream;
  l_stream << "./res/" << m_linkToMarket->getName() << "/Market" << m_identifier << "_OrderBookHistory.csv";
  std::string fileName = l_stream.str();
  std::ofstream outFile(fileName.c_str());
  
  printHeader(outFile);
  
  std::vector<OrderBookHistory>::const_iterator it;
  for(it = m_orderBookHistory.begin(); it != m_orderBookHistory.end(); it++) {
    printLineOfHistory(*it,outFile);
  }
  outFile.close();
}

void OrderBook::printOrderBookHistoryOnTheFly(double a_time) {  
  std::ostringstream l_stream;
  l_stream << "csv" << m_identifier << ".txt";
  std::string fileName = l_stream.str();
  std::ofstream outFile(fileName.c_str());
    
  if(!m_headerPrinted) {
    printHeader(outFile);
    m_headerPrinted = true;
  }
  OrderBookHistory l_line = buildAHistoryLine(a_time);
  printLineOfHistory(l_line,outFile);
  outFile.close(); 
}

void OrderBook::printHeader(std::ofstream& a_outFile) const {
  a_outFile << "date_xl,ts,";
  for(int i = 0; i < m_maxDepth; i++) {
      a_outFile << "bid" << i + 1 << ",bidQ"
		<< i + 1 << ",ask" << i + 1 << ",askQ" << i + 1;
  }
  a_outFile << ",last,lastQ,lastF" << std::endl;
}

int OrderBook::getPrice() const {
  return m_last;
}

void OrderBook::getOrderBookForPlot(std::vector<int>& a_price,
				    std::vector<int>& a_priceQ,
				    std::vector<int>& MMprices,
				    std::vector<int>& MMvolumes) const {
  
  std::map<int, std::list<Order> >::const_iterator itBids; 
  std::map<int, std::list<Order> >::const_iterator itAsks; 
  
  std::vector<LimitOrders> l_vector;
  
  for(itBids = m_bids.begin(); itBids != m_bids.end(); itBids++) {
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
    
    a_priceQ.push_back(-1 * getQuantityForThisPrice(m_bids, l_bid));
  }
  for(itAsks = m_asks.begin(); itAsks != m_asks.end(); itAsks++) {
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
    a_priceQ.push_back(getQuantityForThisPrice(m_asks, l_ask));
  }	
}

std::map<int, std::list<Order> > OrderBook::getBidQueue() const {
  return m_bids;
}

std::map<int, std::list<Order> > OrderBook::getAskQueue() const {
  return m_asks;
}

int OrderBook::getNumberOfOrders(int a_agentIdentifier, int a_price) const {
  int l_bid = getBidPrice();
  int l_ask = getAskPrice();
  int l_number = 0;
  if ((a_price < l_ask) && (a_price > l_bid)) {
    l_number = 0;
  }
  else if (a_price >= l_ask) {
    std::map<int, std::list<Order> >::const_iterator it = m_asks.find(a_price);
    if(it != m_asks.end()) {
      std::list<Order>::const_iterator l_order;
      for (l_order=(*it).second.begin(); l_order!=(*it).second.end(); l_order++) {
	if ((*l_order).m_owner == a_agentIdentifier) l_number++;
      }
    }
  }
  else {
    std::map<int, std::list<Order> >::const_iterator it = m_bids.find(a_price);
    if(it != m_bids.end()) {
      std::list<Order>::const_iterator l_order;
      for (l_order=(*it).second.begin(); l_order!=(*it).second.end(); l_order++) {
	if ((*l_order).m_owner == a_agentIdentifier) l_number++;
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
  // Look for order in m_asks
  itPrice = m_asks.begin();
  while(!orderFoundinAsks && itPrice != m_asks.end()) {
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
  /* If not found, look for order in m_bids */
  if(!orderFoundinAsks) {
    itPrice = m_bids.begin();
    while(!orderFoundinBids && itPrice != m_bids.end()) {
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
	m_bids.erase(itPrice);
      }
      else
	{
	  m_asks.erase(itPrice);
	}
    }
    /* notify owner of order */
    m_linkToMarket->notifyCancellation(a_agentIdentifier,a_orderIdentifier,a_time);
    m_linkToMarket->updateCurrentTime(a_time);
    m_linkToMarket->notifyAllAgents();
  }
  /* ... else raise error */
  else {
    std::cout << "Order not found in OrderBook::processCancellation." << std::endl;
    exit(1);
  }
}

const std::vector<OrderBookHistory>& OrderBook::getOrderBookHistory() const {
  return m_orderBookHistory;
}

const std::vector<Order>& OrderBook::getOrderHistory() const {
  return m_orderHistory;
}

int OrderBook::getTotalBidQuantity() {
  long totalBidQuantity = 0;
  std::map<int, std::list<Order> >::iterator it = m_bids.begin();
  while(it != m_bids.end()) {
    std::list<Order>::iterator l_order = (*it).second.begin();
    while(l_order!=(*it).second.end()) {
      totalBidQuantity += (*l_order).m_volume;
      l_order++;
    }
    it++;
  }
  return totalBidQuantity;
}

int OrderBook::getTotalAskQuantity() {
  long totalAskQuantity = 0;
  std::map<int, std::list<Order> >::iterator it = m_asks.begin();
  while(it != m_asks.end()) {
    std::list<Order>::iterator l_order = (*it).second.begin();
    while(l_order!=(*it).second.end()) {
      totalAskQuantity += (*l_order).m_volume;
      l_order++;
    }
    it++;
  }
  return totalAskQuantity;
}

int OrderBook::getBidPriceAtLevel(int levelMax)
{
  std::map<int, std::list<Order> >::const_reverse_iterator it = m_bids.rbegin();
  int levelCount = 0;
  while(it!=m_bids.rend() && levelCount<levelMax) {
      if(!it->second.empty()) {
	  levelCount++;
      }
      it++;
  }
  if(it == m_bids.rend()) it--;
  return it->first;
}

int OrderBook::getAskPriceAtLevel(int levelMax) {
  std::map<int, std::list<Order> >::const_iterator it = m_asks.begin();
  int levelCount = 0;
  while(it!=m_asks.end() && levelCount<levelMax) {
    if(!it->second.empty()) {
	levelCount++;
    }
    it++;
  }
  if(it == m_asks.end()) it--;
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
    std::map<int, std::list<Order> >::const_iterator it = m_asks.find(a_price);
      if(it != m_asks.end()) {
	std::list<Order>::const_iterator l_order;
	for (l_order=(*it).second.begin();l_order!=(*it).second.end();l_order++) {
	  l_number++;
	}
      }
  }
  else {
    std::map<int,std::list<Order> >::const_iterator it = m_bids.find(a_price);
    if(it != m_bids.end()) {
      std::list<Order>::const_iterator l_order;
      for (l_order=(*it).second.begin(); l_order != (*it).second.end(); l_order++) {
	l_number++;
      }
    }
  }
  return l_number;
}

void OrderBook::cleanOrderBook() {
  m_asks.clear();
  m_bids.clear();
}

void OrderBook::setDefaultBidAsk(int bid, int ask) {
  m_defaultBid = bid;
  m_defaultAsk = ask;
}

std::vector<int> OrderBook::getHistoricPrices() {
  return m_historicPrices;
}

std::vector<double> OrderBook::getTransactionsTimes() {
  return m_transactionsTimes;
}

double OrderBook::getReturnsSumSquared() {
  return m_returnsSumSquared;
}
