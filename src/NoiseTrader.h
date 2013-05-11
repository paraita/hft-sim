#ifndef __NOISETRADER__H__
#define __NOISETRADER__H__

#include "Agent.h"
class Market;
class OrderBook;
class Distribution;

class NoiseTrader : public Agent {

 public:

  NoiseTrader(Market *a_market, 
	      Distribution *a_ActionTimeDistribution,
	      Distribution *a_OrderTypeDistribution,
	      Distribution *a_OrderVolumeDistribution,
	      double a_buyFrequency,
	      int a_stockIdentifier = 1);
  virtual ~NoiseTrader();
  virtual void processInformation();
  virtual double getNextActionTime() const;
  virtual void makeAction(int a_OrderBookId, double currentTime);

 private:
  
  Distribution *m_ActionTimeDistribution;
  Distribution *m_OrderTypeDistribution;
  Distribution *m_OrderVolumeDistribution;
  double m_buyFrequency;
  double m_sellFrequency;
		
  virtual OrderType getOrderType() const;
  virtual int getOrderVolume() const;
  virtual int getOrderPrice (int a_OrderBookId, OrderType a_OrderType) const;
  virtual void submitCancellation(int a_OrderBookId,bool a_cancelBuy) const;

};

#endif //__NOISETRADER__H__
