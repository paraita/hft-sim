#ifndef __TYPES__H__
#define __TYPES__H__

#include <vector>


/*! \brief These are market order types
 *
 *  Classically, we have sell and buy limit orders, as well as sell and buy market orders.
 *  Notice that if a market order is a crossing one, it is well treated.
 */
typedef enum
{
	MARKET_SELL = 0,
	MARKET_BUY,
	LIMIT_SELL,
	LIMIT_BUY,
	CANCEL_BUY,
	CANCEL_SELL,
	CLEAR_OB
} OrderType;

/*! \brief Describes the state of an order
 *
 *  This state order description concerns naturally the limit ordersClassically, we have sell and buy limit orders, as well as sell and buy market orders.
 *  Notice that if a market order is a crossing one, it is well treated.
 */
typedef enum
{
	ALIVE = 0,
	EXECUTED,
	PARTIALLY_EXECUTED,
	CANCELED
} OrderState;

/*! \brief different kinds of traders
 *
 *  This list is evolutive.
 */
typedef enum
{
	DEFAULT = 0,
	LIQUIDITY_PROVIDER,
	LIQUIDITY_PROVIDER_UC,
	NOISE_TRADER,
	LARGE_ORDER_TRADER,
	TREND_FOLLOWER,
	FUNDAMENTAL_VALUE_TRADER,
	MARKET_MAKER
} AgentType;

/*! \brief Storage class for execution history of an order
 *
 *  The execution history of an order is a triplet containing (executionTime,ExecutionPrice,ExecutionQuantity)
 *  The vector is filled in on every event on the order (partial executions), until the order is fully executed or the simulation stops.
 */
class ExecutionHistory
{
	public:
		ExecutionHistory(double a_executionTime,int a_executionQuantity,int a_executionPrice)
		{
			m_executionPrice		= a_executionPrice;
			m_executionQuantity		= a_executionQuantity;
			m_executionTime			= a_executionTime;
			
		}
		int m_executionPrice;
		int m_executionQuantity;
		double m_executionTime;
};

/*! \brief Storage class for limit orders
 *
 *  This storage class will be used in the orderBookHistoryVector, to store or print the limit orderss
 *  until a given depth.
 *
 */
class LimitOrders
{
	public:
		LimitOrders(int a_bid,int a_bidQ,int a_ask,int a_askQ)
		{
			m_bid		= a_bid;
			m_bidQ		= a_bidQ;
			m_ask		= a_ask;
			m_askQ		= a_askQ;
			
		}
		int m_bid;
		int m_bidQ;
		int m_ask;
		int m_askQ;
};
/*! \brief Storage class for the order book history
 *
 *  The members of this class correspond to the headers of the order book history that will be printed in csv files
 *  and treated with Python scripts.
 */
class OrderBookHistory
{
	public:
		OrderBookHistory(double a_datexl,double a_timeStamp, std::vector<LimitOrders> a_limitOrders,int a_last,int a_lastQ)
		{
			m_datexl = a_datexl;
			m_timeStamp = a_timeStamp;
			m_limitOrders = a_limitOrders;
			m_last = a_last;
			m_lastQ = a_lastQ;
		}
		double m_datexl;
		double m_timeStamp;
		std::vector<LimitOrders> m_limitOrders;
		int m_last;
		int m_lastQ;
};
/*! \mainpage Simulator
 *
 * \section intro_sec The project
 *
 * Simulator is a multi-agent generic market engine. It models a market with several assets, and with several agents acting on these assets.
 * The market is order driven and the agents are able to push two kinds of orders : limit and market orders, and to cancel previously pushed limit orders.
 * For every asset, there is by default a liquidity provider always pushing limit orders
 * on that asset and a noise trader always pushing market orders on that asset. So by default, the market is composed of n assets evolving independently.
 * You can create your own agents, decide of a strategy to enable them act on several assets, test strategies, print order book history, have access to agents' cash position and so on...
 * The engine enables you to charge a market fee to your agent, so as to take into account the transaction costs while developing your strategies.
 *
 * \section install_sec Getting started
 *
 * Just compile the code, and execute it as it is. You will have a simulation of a market with pre-coded zero intelligence agents.
 * You can modify the parameters of the simulation in the file Simulator.cpp, by modifying the values of the global variables set before the main() function.
 *  
 * \section next_sec TO DO's
 *
 * Design a user friendly interface to enter the parameters.
 *
 *  
 *
 */


#endif // __TYPES__H__
