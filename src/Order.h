#ifndef __ORDER__H__
#define __ORDER__H__

#include <vector>
#include "Types.h"

/*! \brief Generic Order class
 *
 *  The class implements different market mechanisms : storing, execution, cancellation of orders and so on...
 *
 */
class Order
{
	friend class Agent;
	friend class OrderBook;
	friend class Market;
	public:
		Order();
		/*! \brief Generic Agent constructor
		 *
		 *  \param a_asset asset identifier
		 *  \param a_owner agent identifier
		 *  \param a_time time stamp
		 *  \param a_price price (for limit orders)
		 *  \param a_volume quantity
		 *  \param a_type order type
		 *  \param a_globalOrderIdentifier given by the market : a unique order identifier 
		 *
		 */
		Order(int a_asset , int a_owner , double	a_time , int a_price,
					int	a_volume , OrderType a_type ,
					int a_globalOrderIdentifier, OrderState state);

		virtual ~Order();

		int getUnderlying() ;
		int getIdentifier() ;
		int getOwner() ;
		int getVolume() ;
		int getInitialVolume() ;
		int getPrice() ;
		double getTime() ;
		OrderType getType() ;
		void printOrder() ;
		
		void setPrice (int a_price) ;
		void setVolume(int a_volume);
		void setType(OrderType a_type) ;

		void setState(OrderState state);
		OrderState getState();

		int getPriority();
		void setPriority(int priority);

	private:
		
		int				m_asset;
		int				m_owner;
		double			m_time;
		int				m_price;
		int				m_initialVolume;
		int				m_volume;
		OrderType		m_type;
		OrderState		m_state;
		int				m_globalOrderIdentifier;

		double			m_newBid;
		double			m_newAsk;

		int				m_priority;

		std::vector < ExecutionHistory> m_executionHistory;

};

#endif // __ORDER__H__
