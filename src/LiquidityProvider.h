#ifndef LIQUIDITYPROVIDER_UC_H_
#define LIQUIDITYPROVIDER_UC_H_

#include "Agent.h"

class Market;
class OrderBook;
class Distribution;
class DistributionUniform;

class LiquidityProvider : public Agent {
	
 public:

  LiquidityProvider(Market * a_market, 
		    Distribution * a_ActionTimeDistribution,
		    Distribution * a_OrderVolumeDistribution,
		    Distribution * a_OrderPriceDistribution,
		    double a_buyFrequency,
		    int a_favouriteStockIdentifier,
		    double a_cancelBuyFrequency,
		    double a_cancelSellFrequency,
		    double a_cancelProbability
		    );

  virtual ~LiquidityProvider() ;
		
 public:

  virtual double getNextActionTime() const;
  virtual void makeAction(int a_OrderBookId, double currentTime);	
  virtual void makeAction(int a_OrderBookId, double currentTime, bool init);	 
  virtual void processInformation();
  void cleanPending();

 protected:
  
  virtual OrderType getOrderType() const;
  virtual int getOrderVolume() const;
  virtual int getOrderVolume(double price, int a_OrderBookId , OrderType orderType) const ;
  virtual int getOrderPrice (int a_OrderBookId, OrderType a_OrderType) const;
  void chooseOrdersToBeCanceled(int a_OrderBookId, bool a_buySide, double a_time);

 private:
		
  double m_cancelBuyFrequency;
  double m_cancelSellFrequency;
  double m_buyFrequency ;
  double m_sellFrequency;
  double m_cancelProbability ;
  DistributionUniform *m_cancelDistribution ;
  Distribution *m_ActionTimeDistribution ;
  Distribution *m_OrderVolumeDistribution ;
  Distribution *m_OrderPriceDistribution ;
  DistributionUniform *m_OrderTypeDistribution ;

};

#endif /*LIQUIDITYPROVIDER_UC_H_*/
