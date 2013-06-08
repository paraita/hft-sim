#ifndef __MARKET__H__
#define __MARKET__H__

#include <map>
#include <string>
#include "Types.h"

class NewsServer;
class Agent;
class OrderBook;
class Order;
class RandomNumberGenerator;
class Distribution;

/*! \brief Generic Market class
 *
 *  The class implements different market mechanisms : storing, execution, cancellation of orders and so on...
 *
 */
class Market
{
	public:
		Market(std::string a_marketName);
		virtual ~Market();
		
		/*! \brief Create a_number assets to be traded on the market.
		 *
		 *  \param a_number number of traded assets
		 *  \param a_tickSize tick size
		 *  \param a_defaultBid it is only needed to put the first limit bid orders, as these are put in a certain distance relative to the bid price. So the order book cannot be empty at the beginning of the simulation
		 *  \param a_defaultAsk same as for bids
		 */
		void createAssets(int a_number = 1, int a_tickSize = 1,int a_defaultBid = 9999, int a_defaultAsk = 10000);

		/*! \brief Agent registration
		 *
		 *  \param a_Agent link to the registered agent
		 */
		void registerAgent(Agent * a_Agent) ;

		/*! \brief Sends an order to its order book, where it will be processed
		 *
		 *  \param a_order order to be processed
		 */
		void pushOrder(Order &a_order);

		/*! \brief updates the current time in the simulation
		 *
		 */
		void updateCurrentTime(double a_time);
		
		/*! \brief returns the current time in the simulation
		 *
		 */
		double getCurrentTime();

		/*! \brief returns an order identifier
		 *
		 */
		int getOrderIdentifier();

		/*! \brief Order execution notification
		 *
		 *  \param a_agentIdentifier identifier of the agent to be notified
		 *  \param a_orderIdentifier Order identifier.
		 *  \param a_time time stamp
		 *  \param a_price price of execution
		 *
		 */
		void notifyExecution(int a_agentIdentifier,int a_orderIdentifier,double a_time,int a_price);

		/*! \brief partial order execution notification
		 *
		 *  \param a_agentIdentifier identifier of the agent to be notified
		 *  \param a_orderIdentifier Order identifier.
		 *  \param a_time time stamp
		 *  \param a_volume quantity executed
		 *  \param a_price price of execution
		 *
		 */
		void notifyPartialExecution(int a_agentIdentifier,int a_orderIdentifier,double a_time,int a_volume,int a_price);

		/*! \brief order cancellation notification
		 *
		 *  \param a_agentIdentifier identifier of the agent to be notified
		 *  \param a_orderIdentifier Order identifier.
		 *  \param a_time time stamp
		 */
		void notifyCancellation(int a_agentIdentifier,int a_orderIdentifier,double a_time);
		
		/*! \brief gets an agent by its identifier
		 *
		 *  \param a_agentIdentifier identifier of the agent
		 *
		 */
		Agent * getAgent(int a_agentIdentifier);

		/*! \brief gets an asset by its identifier
		 *
		 *  \param a_assetIdentifier identifier of the asset
		 *
		 */
		OrderBook* getOrderBook(int a_assetIdentifier);
		
		/*! \brief Notifies all agents of every order processing on the order book
		 *
		 */
		void notifyAllAgents();
		
		/*! \brief sets the next action time and the next acting agent
		 *
		 */
		void setNextActionTime();
		
		/*! \brief gets the next acting agent (whose next action time is minimum)
		 *
		 */
		Agent* getNextActor();

		/*! \brief gets the next action time
		 *
		 */
		double getNextActionTime();

		/*! \brief calculate the fees charged by the market
		 *
		 *  \param a_price execution price of the order
		 *  \param a_volume executed volume
		 *  \param a_orderType type  of the order, as we may decide that fees are order-type-dependent
		 *  \param a_agentIdentifier identifier of the agent as we may decide that fees depend on the agent
		 *
		 */
		double calculateMarketFees(int a_price,int a_volume,OrderType a_orderType, int a_agentIdentifier);
		
		/*! \brief Get the market random number generator
		 * 	\return Pointer of type gsl_rng *
		 */
		RandomNumberGenerator * getRNG() ;

		/*! \brief Get the name of the market
		 * 	\return std::string object
		 */
		std::string getName() ;
		
		/*! \brief creates a news Server
		 *
		 *  \param a_newsTimeDistribution arrival time of the news
		 *	\param a_newsQualityDistribution quality of the news
		 */
		void createNewsServer(Distribution *a_newsTimeDistribution, Distribution *a_newsQualityDistribution);

	private:

		/// Name of market ; used for naming the output directory ./res/m_marketName/
		std::string m_marketName ;
	
		RandomNumberGenerator * m_RNG ;
	
		double m_currentTime;

		std::map<int,Agent*> m_agents;
		int m_agentCounter;
		std::map<int,double> m_NextActionTime;
		double m_nextActionTime;
		int m_nextActorId;

		std::map<int,OrderBook*> m_assetOrderBooks;
		int m_assetCounter;

		int m_globalOrderCounter;
		NewsServer * m_newsServer;
		double m_news;
};

#endif // __MARKET__H__
