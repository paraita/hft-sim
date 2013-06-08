#ifndef __NOISETRADER__H__
#define __NOISETRADER__H__

#include "Agent.h"
class Market;
class OrderBook;
class Distribution;

/*! \brief Noise trader class
 *
 *  The class implements a noise trader. This kind of trader submits market buy and sell orders. The waiting time between two 
 *  consecutive orders as well as the order kind and order volule are random, obeying a certain distribution specified when the
 *	agent is created.
 *
 */
class NoiseTrader : public Agent 
{

	public:
		/*! \brief noise trader constructor
		 *
		 *  \param a_market a link to the implemented market.
		 *  \param a_ActionTimeDistribution Probability distribution of waiting time between two actions.
		 *  \param a_OrderTypeDistribution Probability distribution of order type.
		 *  \param a_OrderVolumeDistribution Probability distribution of order volume.
		 *  \param a_buyFrequency Percecntage of buy orders.
		 *
		 */
		NoiseTrader(Market * a_market, 
			Distribution * a_ActionTimeDistribution,
			Distribution * a_OrderTypeDistribution,
			Distribution * a_OrderVolumeDistribution,
			double a_buyFrequency,
			int a_stockIdentifier = 1
			) ;
		virtual ~NoiseTrader();
		
		/*! \brief processes information after the agent is notified of a market event
		 *
		 * This method can be used to implement a feedback mechanism. After notification of a market event, this method is called to allow the agent to react to the market event.
		 */
		virtual void processInformation();
		
		
		
		/*! \brief gets action time for the agent
		 *
		 *  This distribution of waiting time beween two consecutive actions by the trader is specified by the constructor.
		 */
		virtual double getNextActionTime() const;

		/*! \brief Action : can only be an order submission for a noise trader
		 *
		 * \param a_linkToMarket link to the market
		 * \param a_OrderBookId asset identifier
		 * \param currentTime time stamp
		 *
		 */
		virtual void makeAction(int a_OrderBookId, double currentTime);

	private:
		Distribution * m_ActionTimeDistribution ;
		Distribution * m_OrderTypeDistribution ;
		Distribution * m_OrderVolumeDistribution ;
		double m_buyFrequency;
		double m_sellFrequency;
		

		/*! \brief returns the order type
		 *
		 * The order type is generated randomly and can be a market sell or a market buy order.
		 */	
		virtual OrderType getOrderType() const;

		/*! \brief returns the order volume
		 *
		 * The order volume is generated randomly according to a specified distribution.
		 */
		virtual int getOrderVolume() const;
		/*! \brief without effect
		 *
		 * This method does not have any role here as all the orders submitted by noise traders are market orders.
		 */
		virtual int getOrderPrice (int a_OrderBookId, OrderType a_OrderType) const;	

		/*! \brief without effect
		 *
		 * This method does not have any role here as all the orders submitted by noise traders are market orders.
		 */
		virtual void submitCancellation(int a_OrderBookId,bool a_cancelBuy) const;
	
	
} ;

#endif //__NOISETRADER__H__
