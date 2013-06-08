#ifndef LIQUIDITYPROVIDER_UC_H_
#define LIQUIDITYPROVIDER_UC_H_

#include "Agent.h"

class Market;
class OrderBook;
class Distribution;
class DistributionUniform;


/*! \brief Liquidity provider class
 *
 *  The class implements a liquidity provider with a uniform cancelation process
 */
class LiquidityProvider : public Agent {
	
	public:
		/*! \brief Liquidity provider constructor
		 *
		 *  \param a_market a link to the implemented market.
		 *  \param a_ActionTimeDistribution Probability distribution of waiting time between two actions.
		 *  \param a_OrderTypeDistribution Probability distribution of order type.
		 *  \param a_OrderVolumeDistribution Probability distribution of order volume.
		 *  \param a_OrderPriceDistribution Probability distribution of limit order price.
		 *  \param a_cancellationFrequency Frequency of cancellations in percentage.
		 *  \param a_buyFrequency Frequency of buy orders in percentage.
		 *  cancellationFrequency+buyFrequency+sellFrequency = 1
		 */
		LiquidityProvider(Market * a_market, 
			Distribution * a_ActionTimeDistribution,
			Distribution * a_OrderVolumeDistribution,
			Distribution * a_OrderPriceDistribution,
			double a_buyFrequency,
			int a_favouriteStockIdentifier,
			double a_cancelBuyFrequency,
			double a_cancelSellFrequency,
			double a_cancelProbability
			) ;
		virtual ~LiquidityProvider() ;
		
	public:
		/*! \brief gets action time for the agent
		 *
		 *  This distribution of waiting time beween two consecutive actions by the trader is specified by the constructor.
		 */
		virtual double getNextActionTime() const;
		/*! \brief Action : can be an order submission or a cancellation
		 *
		 * \param a_linkToMarket link to the market
		 * \param a_OrderBookId asset identifier
		 * \param currentTime time stamp
		 *
		 */
		virtual void makeAction(int a_OrderBookId, double currentTime);	
		/*! \brief processes information after the agent is notified of a market event
		 *
		 * This method can be used to implement a feedback mechanism. After notification of a market event, this method is called to allow the agent to react to the market event.
		 */
		virtual void processInformation();

		void cleanPending();

		void printPending();

		void makeBuyAction(int a_OrderBookId, double a_currentTime);

		void makeSellAction(int a_OrderBookId, double a_currentTime);



	protected:
		/*! \brief returns the order type
		 *
		 * The order type is generated randomly and can be a limit sell or a limit buy order, or a cancelation of existing orders.
		 */	
		virtual OrderType getOrderType() const;
		/*! \brief returns the order volume
		 *
		 * The order volume is generated randomly according to a specified distribution.
		 */
		virtual int getOrderVolume() const;

		/*! \brief returns the order volume with respect to the input price
		 *
		 * The order volume is generated randomly according to a specified distribution depending on the price.
		 */

		virtual int getOrderVolume(double price, int a_OrderBookId , OrderType orderType) const ;

		/*! \brief returns the order price
		 *
		 * The limit order price is generated randomly according to a specified distribution.
		 */
		virtual int getOrderPrice (int a_OrderBookId, OrderType a_OrderType) const;

		/*! \brief go through the order book and cancel order with uniform probability
		 *
		 */
		void chooseOrdersToBeCanceled(int a_OrderBookId, bool a_buySide, double a_time);

		
		
	private:
		
		double m_cancelBuyFrequency;
		double m_cancelSellFrequency;
		double m_buyFrequency ;
		double m_sellFrequency;
		double m_cancelProbability ;
		
		DistributionUniform * m_cancelDistribution ;
		/// Distribution of the interval times between two consecutive actions
		Distribution * m_ActionTimeDistribution ;
		/// Distribution of the volume of the orders
		Distribution * m_OrderVolumeDistribution ;
		/// Distribution of the lag price relative to the opposite best limit when submitting a limit order
		Distribution * m_OrderPriceDistribution ;

		DistributionUniform * m_OrderTypeDistribution ;

};

#endif /*LIQUIDITYPROVIDER_UC_H_*/
