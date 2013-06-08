#ifndef __AGENT__H__
#define __AGENT__H__

#include <map>
#include <vector>
#include "Types.h"
#include <concurrent_unordered_map.h>

#include <boost\thread\mutex.hpp>

class Order;
class Market;

/*! \brief Generic Agent class
 *
 *  The class specifies an interface of a generic agent. This is an abstract class. You have to inherit from it and to overload virtual member functions
 *  in order to create your own agents.
 *
 */
class Agent
{
	friend class Market;
	public:
		
		/*! \brief Generic Agent constructor
		 *
		 *  \param a_market a link to the implemented market.
		 *  \param a_favouriteStockId The stock on which the agent acts (-1 if the agent can act on all the stocks)
		 *  \param a_Type type of the agent.
		 *
		 */
		Agent(Market * a_market, AgentType a_Type = DEFAULT,int a_favouriteStockId = 1);
	
		
		virtual ~Agent();
		
		/*! \brief Order execution notification
		 *
		 *  \param a_orderIdentifier Order identifier.
		 *  \param a_time time stamp
		 *  \param a_price price of execution
		 *
		 *	This member function is called by the market to notify the agent that his order was executed
		 */
		void notifyExecution(int a_orderIdentifier,double a_time,int a_price);

		/*! \brief partial order execution notification
		 *
		 *  \param a_orderIdentifier Order identifier.
		 *  \param a_time time stamp
		 *  \param a_volume quantity executed
		 *  \param a_price price of execution
		 *
		 *	This member function is called by the market to notify the agent that his order was partially executed
		 */
		void notifyPartialExecution(int a_orderIdentifier,double a_time,int a_volume,int a_price);

		/*! \brief order cancellation notification
		 *
		 *  \param a_orderIdentifier Order identifier.
		 *  \param a_time time stamp
		 *
		 *	This member function is called by the market to notify the agent that his order was canceled
		 */
		void notifyCancellation(int a_orderIdentifier,double a_time);

		/*! \brief Cash position calculation, and Cash position history update
		 *
		 *  \param a_time time stamp
		 *  \param a_volume quantity executed
		 *  \param a_price execution price
		 *  \param a_type order type
		 *
		 *	This member function is automatically called by the market after every event on the agent's orders.
		 *  Be careful : If you want to end your simulation and calculate the agent's cash position, then let him send a market order
		 *  with a zero quantity to force the call of this function, and the update of the cash position
		 */
		virtual void processCashPosition(double a_time,int a_volume,int a_price,const OrderType &a_type);

		/*! \brief sets the agent identifier
		 *
		 *  \param a_identifier Agent identifier.
		 *
		 *	This member function is called by the market to notify the agent that his order was canceled
		 */
		void setIdentifier(int a_identifier) ;

		/*! \brief gets the agent type
		 *
		 */
		AgentType getAgentType() const;
		
		
		
		/*! \brief gets action time for the agent
		 *
		 *  This is a pure virtual function. You have to overload and implement it in your agent class.
		 */
		virtual double getNextActionTime() const = 0 ;
		
		/*! \brief Action : can be an order submission or a cancellation
		 *
		 * \param a_OrderBookId asset identifier
		 * \param currentTime time stamp
		 *
		 *  This is a pure virtual function. You have to overload and implement it in your agent class.
		 */
		virtual void makeAction(int a_OrderBookId, double currentTime) = 0 ;
		

		/*! \brief processes information after the agent is notified of a market event
		 *
		 * This method can be used to implement a feedback mechanism. After notification of a market event, this method is called to allow the agent to react to the market event.
		 */
		virtual void processInformation() = 0;
		
		/*! \brief Market event notification by the market
		 *
		 *  After every order processing, the market informs the agents that an event took place by this fucntion.
		 */
		void notifyAgentOfMarketEvent();
		
		/*! \brief inits to 0 the possessed quantity of the concerned asset
		 *
		 *  \param a_stockIdentifier
		 */
		void initAssetQuantity(int a_stockIdentifier);

		/*! \brief returns the quantity of concerned stock
		 *
		 *  \param a_stockIdentifier
		 */
		int getStockQuantity(int a_stockIdentifier);

		/*! \brief returns the id of the stock on which the trader acts, -1 if the trader can act on all the stocks.
		 *
		 */
		int getFavouriteStock();
		
		/*! \brief returns the syock concerned with current action
		 *
		 */
		virtual int getTargetedStock();
		
		/*! \brief returns the fee rate charged by the market
		 * \param a_orderType The fee rate can depend on the order type
		 */
		virtual double getFeeRate(OrderType a_orderType);
		
		/*! \brief charges a market fee to the agent
		 * 
		 */
		void chargeMarketFees(double a_fee);

		/*! \brief returns the Cash Position net of market fees
		 * 
		 */
		double getNetCashPosition();
		
		/*! \brief returns reference to map of pending orders
		 * 
		 */
		concurrency::concurrent_unordered_map<int,Order> * getPendingOrders() ;

		boost::mutex mtx_;

		int Agent::nbPendingOrder();
	
	protected:
		int	m_identifier;
		AgentType m_Type ;
		Market * m_linkToMarket; // DO NOT DELETE, JUST A LINK

		concurrency::concurrent_unordered_map<int,Order> m_passedOrders;
		
		/*! \brief map of pending orders (i.e. orders which can be cancelled)
		 * 
		 */
		concurrency::concurrent_unordered_map<int,Order> m_pendingOrders;
		
		int m_cashPosition;
		
		std::vector <std::pair <double,int> > m_cashPositionHistory;//<time,cash>
		

		std::map<int,int> m_stockQuantity;

		int m_favouriteStockId;
		double m_fees;
		
		void updateStockNumber(Order *a_order,int a_executedVolume);
		bool isMine(Order a_order) const;

		/*! \brief Order submission
		 *
		 *  \param a_asset Asset identifier.
		 *  \param a_time Time stamp
		 *  \param a_volume volume, or quantity
		 *  \param a_type type of the order
		 *  \param a_price This is used only for limit orders
		 *
		 */
		
		void submitOrder(int a_asset, double a_time, int a_volume, OrderType a_type, int a_price = 0, int priority = 0);
		/*! \brief returns the order type
		 *
		 */	
		virtual OrderType getOrderType() const = 0;

		/*! \brief returns the order volume
		 *
		 */
		virtual int getOrderVolume() const = 0;

		/*! \brief returns the order price
		 *
		 */
		virtual int getOrderPrice (int a_OrderBookId, OrderType a_OrderType) const = 0;


		/*! \brief submits a cancellation demand to the market
		 *
		 */
		void submitCancellation(int a_OrderBookId,int a_orderIdentifier, int a_price, double a_time , OrderType type);

};

#endif // __AGENT__H__
