#ifndef __ORDERBOOK__H__
#define __ORDERBOOK__H__

#include <map>
#include <list>
#include <cstdio>

#include <concurrent_queue.h>
#include <concurrent_unordered_map.h>
#include <concurrent_vector.h>
#include <concurrent_priority_queue.h>

#include "Order.h"
#include "OrderPriority.h"

//class Order;
class Market;
/*! \brief Generic order book class
*
*  The class implements an order book, on a given asset.
*
*/
class OrderBook
{
public:
	/*! \brief order book constructor
	*
	*  \param a_market link to the market
	*  \param a_identifier asset identifier
	*  \param a_tickSize tick size
	*  \param a_defaultBid it is only needed to put the first limit bid orders, as these are put in a certain distance relative to the bid price. So the order book cannot be empty at the beginning of the simulation
	*  \param a_defaultAsk same as for bids
	*
	*/
	OrderBook(Market *a_market, int a_identifier, int a_tickSize, int a_defaultBid, int a_defaultAsk);
	virtual ~OrderBook();



	int getIdentifier() const ;

	/*! \brief returns the ask price
	*
	*/
	int getAskPrice();

	/*! \brief returns the bid price
	*
	*/
	int getBidPrice();

	/*! \brief returns the tick size
	*
	*/
	int getTickSize() const;

	/*! \brief returns the total quantity of the best bid price
	*
	*/
	int getBidQuantity();

	/*! \brief returns the total quantity of the best ask price
	*
	*/
	int getAskQuantity();

	/*! \brief returns the available quantity for a given limit price
	*
	*  \param a_orderQueue The order queue (the bid one, or the ask one)
	*  \param a_priceLevel price level
	*
	*  If the price does not exist, it returns 0.
	*/
	int getQuantityForThisPrice(int a_priceLevel);

	///*! \brief returns the available quantity up to a given Bid price
	// */
	//int getBidQuantityUpToPrice(int a_price) const;
	//
	///*! \brief returns the available quantity up to a given ask price
	// */
	//int getAskQuantityUpToPrice(int a_price) const;
	//


	/*! \brief processes the order
	*
	*  \param a_order order to be processed
	*
	*  The function also notifies the owner of the order of the result of this processing, and updates the last price for the concerned asset, and the cash position for concerned agents.
	*/
	void runOrderBook();

	/*! \brief prints an already stored history of the order book, if the bolean m_storeOrderBookHistory was set to true
	*
	*/
	void printStoredOrderBookHistory();

	/*! \brief sets the storage choice
	*
	*  \param a_store true if you want to remember all the events at a certain depth
	*  \param a_depth maximum depth.
	*  
	*/
	void setStoreOrderBookHistory(bool a_store, int a_depth);
	void setStoreOrderHistory(bool a_store);

	/*! \brief sets the print choice
	*
	*  \param a_print true if you want to print events when they come
	*  \param a_depth maximum depth.
	*
	*/
	void setPrintOrderBookHistory(bool a_print, int a_depth);

	/*! \brief returns the last price of the asset
	*
	*/
	int getPrice() const;

	/*! \brief returns the distance to best opposite quote in ticks
	*
	*/
	int getDistanceToBestOppositeQuote(int a_price);

	/*! \brief prepares data to be plotted in Gnuplot
	*
	*/
	void OrderBook::getOrderBookForPlot(std::vector<int> &a_price, std::vector<int> &a_priceQ, std::vector<int> &a_priceMM, std::vector<int> &a_priceQMM);

	/*! \brief returns the bid Queue
	*
	*/

	concurrency::concurrent_vector<int> getHistoricPrices();

	concurrency::concurrent_vector<double> getTransactionsTimes();

	double getReturnsSumSquared();

	/*! \brief returns the number of orders of the concerned agent at the specified price
	*
	*/
	//int getNumberOfOrders(int a_agentIdentifier, int a_price) const;
	//int getTotalVolumeAtPrice(int a_price) const;
	int getTotalAskQuantity() ;
	int getTotalBidQuantity() ;
	//int getBidPriceAtLevel(int levelMax) ;
	//int getAskPriceAtLevel(int levelMax) ;

	const std::vector<OrderBookHistory>  & getOrderBookHistory() const;
	const std::vector<Order>  & getOrderHistory() const;



	Market * getLinkToMarket() ;

	void cleanOrderBook();

	void setDefaultBidAsk(int bid, int ask);

	void pushOrder(Order &order);

	void OrderBook::closeOrderBook();

	void quickSort(std::vector<int> &price, std::vector<int> &quantity, int left, int right);

	void activateHFTPriority(bool activate);

	bool open;

	int getNbOrder();

	double getQuantityExchanged();

	int getNbOrderMM();
	

private:
	/*! cancel a specific given order
	* 
	*/
	void processSellCancellation(int a_agentIdentifier,int a_orderIdentifier,  int a_price, double a_time) ;

	void processBuyCancellation(int a_agentIdentifier,int a_orderIdentifier, int a_price, double a_time) ;


	void printOrderBookHistoryOnTheFly(double a_time);
	void storeOrderBookHistory(double a_time);
	OrderBookHistory buildAHistoryLine(double a_time) const;
	void printLineOfHistory(OrderBookHistory a_line,std::ofstream & a_outFile);
	void processLimitBuyOrder(Order  &a_order);
	void processLimitSellOrder(Order  &a_order);
	void processMarketBuyOrder(Order  &a_order);
	void processMarketSellOrder(Order  &a_order);
	void printHeader(std::ofstream & a_outFile) const;

	void updateAskPrice() ;
	void updateBidPrice() ;

	std::map< int , std::list < Order > > getBidQueue() const;

	/*! \brief returns the ask Queue
	*
	*/
	std::map< int , std::list < Order > > getAskQueue() const;



private:
	int m_identifier;
	Market * m_linkToMarket; // DO NOT DELETE, JUST A LINK

	int m_tickSize ;
	int m_defaultBid;
	int m_defaultAsk;
	int m_last;
	int m_lastQ;

	// We need sometimes to iterate on the elements.
	// And while canceling, we can cancel an order situated in the middle of the container
	// hence the choice of a list
	// But Only queue-type operations of lists will be used when pushing (push_back), and executing(pop_front)
	std::map< int , std::list < Order > > m_bids;
	std::map< int , std::list < Order > > m_asks;

	// Parameters for keeping history of orders
	std::vector<OrderBookHistory> m_orderBookHistory;
	std::vector<Order> m_orderHistory;
	bool m_storeOrderBookHistory;
	bool m_storeOrderHistory ;
	bool m_printHistoryonTheFly;
	int m_maxDepth;
	bool m_headerPrinted;

	//contains the prices of the asset
	concurrency::concurrent_vector <int> m_historicPrices;
	//contains the times of transactions (Market orders)
	concurrency::concurrent_vector <double> m_transactionsTimes;


	concurrency::concurrent_priority_queue <Order, OrderPriority> orders;
	concurrency::concurrent_unordered_map<int, int> quantity;
	// A map of map containing all prices and volumes for each agent. The first key is the agent ID and the second the price.
	concurrency::concurrent_unordered_map<int, concurrency::concurrent_unordered_map<int, int>> agentsOrders;
	//concurrency::concurrent_unordered_map<int, int> asks_quantity;

	long totalAskQuantity;
	long totalBidQuantity;

	double m_returnsSumSquared;

	bool m_activateHFTPriority;

	int bid;
	int ask;

	int nbOrder;
	int nbOrderMM;

	double quantityExchanged;

};
template <typename K, typename V>
V get_value_map(const  concurrency::concurrent_unordered_map <K,V> & m, const K & key, const V & defval ) {
	typename concurrency::concurrent_unordered_map<K,V>::const_iterator it = m.find( key );
	if ( it == m.end() ) {
		return defval;
	}
	else {
		return it->second;
	}
}
#endif // __ORDERBOOK__H__
