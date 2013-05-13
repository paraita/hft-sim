#ifndef __ORDERBOOK__H__
#define __ORDERBOOK__H__

#include <map>
#include <list>
#include <cstdio>
class Order;
class Market;

class OrderBook {

 public:

  OrderBook(Market *a_market, int a_identifier, int a_tickSize, int a_defaultBid, int a_defaultAsk);
  virtual ~OrderBook();

  int getIdentifier() const;
  int getAskPrice() const;
  int getBidPrice() const;
  int getTickSize() const;
  int getBidQuantity() const;
  int getAskQuantity() const;
  int getQuantityForThisPrice(std::map<int, std::list<Order> > a_orderQueue, int a_priceLevel) const;
  int getBidQuantityUpToPrice(int a_price) const;
  int getAskQuantityUpToPrice(int a_price) const;
  void processCancellation(int a_agentIdentifier,int a_orderIdentifier, double a_time) ;
  void processOrder(Order& a_order);
  void printStoredOrderBookHistory();
  void setStoreOrderBookHistory(bool a_store, int a_depth);
  void setStoreOrderHistory(bool a_store);
  void setPrintOrderBookHistory(bool a_print, int a_depth);
  int getPrice() const;
  int getDistanceToBestOppositeQuote(int a_price) const;
  void getOrderBookForPlot(std::vector<int>& a_price, std::vector<int>& a_priceQ, std::vector<int>& MMprices, std::vector<int>& MMvolumes) const;
  std::map<int, std::list<Order > > getBidQueue() const;
  std::map<int, std::list<Order > > getAskQueue() const;
  std::vector<int> getHistoricPrices();
  std::vector<double> getTransactionsTimes();
  double getReturnsSumSquared();
  int getNumberOfOrders(int a_agentIdentifier, int a_price) const;
  int getTotalVolumeAtPrice(int a_price) const;
  int getTotalAskQuantity();
  int getTotalBidQuantity();
  int getBidPriceAtLevel(int levelMax);
  int getAskPriceAtLevel(int levelMax);
  const std::vector<OrderBookHistory>& getOrderBookHistory() const;
  const std::vector<Order>& getOrderHistory() const;
  Market* getLinkToMarket();
  void cleanOrderBook();
  void setDefaultBidAsk(int bid, int ask);
		
 private:

  void printOrderBookHistoryOnTheFly(double a_time);
  void storeOrderBookHistory(double a_time);
  OrderBookHistory buildAHistoryLine(double a_time) const;
  void printLineOfHistory(OrderBookHistory a_line, std::ofstream& a_outFile);
  void processLimitBuyOrder(Order& a_order);
  void processLimitSellOrder(Order& a_order);
  void processMarketBuyOrder(Order& a_order);
  void processMarketSellOrder(Order& a_order);
  void printHeader(std::ofstream& a_outFile) const;

 private:

  int identifier;
  Market * linkToMarket; // DO NOT DELETE, JUST A LINK
  int tickSize ;
  int defaultBid;
  int defaultAsk;
  int last;
  int lastQ;

  // We need sometimes to iterate on the elements.
  // And while canceling, we can cancel an order situated in the middle of the container
  // hence the choice of a list
  // But Only queue-type operations of lists will be used when pushing (push_back), and executing(pop_front)
  std::map<int, std::list<Order> > bids;
  std::map<int, std::list<Order> > asks;

  // Parameters for keeping history of orders
  std::vector<OrderBookHistory> orderBookHistory;
  std::vector<Order> orderHistory;
  bool b_storeOrderBookHistory;
  bool storeOrderHistory ;
  bool printHistoryonTheFly;
  int maxDepth;
  bool headerPrinted;

  //contains the prices of the asset
  std::vector<int> historicPrices;

  //contains the times of transactions (Market orders)
  std::vector<double> transactionsTimes;

  double returnsSumSquared;

};

#endif // __ORDERBOOK__H__
