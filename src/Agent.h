/*
 * Agent.h
 *
 *  Created on: 23 avr. 2013
 *      Author: "Paraita Wohler <milkyboi@gmail.com"
 */

#ifndef AGENT_H_
#define AGENT_H_

#include <map>

class Order;
class Market;

class Agent {

	friend class Market;

public:
	Agent(Market *a_market,
	      AgentType a_type = DEFAULT,
	      int a_favouriteStockId = 1);

	virtual ~Agent();

	void notifyExecution(int a_orderIdentifier,
			     double a_time,
			     int a_price);
	
	void notifyPartialExecution(int a_orderIdentifier,
				    double a_time,
				    int a_volume,
				    int a_price);

	void notifyCancellation(int a_orderIdentifier,
				double a_time);

	virtual void processCashPosition(double a_time,
					 int a_volume,
					 int a_price,
					 const OrderType& a_type);

	void setIdentifier(int a_identifier);

	AgentType getAgentType() const;

	virtual double getNextActionTime() const = 0;

	virtual void makeAction(int a_OrderBookId,
				double currentTime) = 0;

	virtual void processInformation() = 0;

	void notifyAgentOfMarketEvent();

	void initAssetQuantity(int a_stockIdentifier);

	int getStockQuantity(int a_stockIdentifier);

	int getFavouriteStock();

	virtual int getTargetedStock();

	virtual double getFeeRate(OrderType a_orderType);

	void chargeMarketFees(double a_fee);

	double getNetCashPosition();

	std::map<int,Order>* getPendingOrders();

protected:
	int	m_identifier;
	AgentType m_Type;
	Market *m_linkToMarket;
	std::map<int,Order> m_passedOrders;
	std::map<int,Order> m_pendingOrders;
	int m_cashPosition;
	std::vector<std::pair<double,int> > m_cashPositionHistory;
	std::map<int,int> m_stockQuantity;
	int m_favouriteStockId;
	double m_fees;

	void updateStockNumber(Order *a_order,int a_executedVolume);
	bool isMine(Order a_order) const;
	void submitOrder(int a_asset, double a_time, int a_volume, OrderType a_type, int a_price = 0);
	virtual OrderType getOrderType() const = 0;
	virtual int getOrderVolume() const = 0;
	virtual int getOrderPrice (int a_OrderBookId, OrderType a_OrderType) const = 0;
	void submitCancellation(int a_OrderBookId,int a_orderIdentifier, double a_time);

};

#endif /* AGENT_H_ */
