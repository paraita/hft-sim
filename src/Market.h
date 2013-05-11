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

class Market {

 public:
  
  Market(std::string a_marketName);
  virtual ~Market();
  void createAssets(int a_number = 1, int a_tickSize = 1, int a_defaultBid = 9999, int a_defaultAsk = 10000);
  void registerAgent(Agent *a_Agent);
  void pushOrder(Order &a_order);
  void updateCurrentTime(double a_time);
  double getCurrentTime();
  int getOrderIdentifier();
  void notifyExecution(int a_agentIdentifier, int a_orderIdentifier, double a_time, int a_price);
  void notifyPartialExecution(int a_agentIdentifier, int a_orderIdentifier, double a_time, int a_volume, int a_price);
  void notifyCancellation(int a_agentIdentifier, int a_orderIdentifier, double a_time);
  Agent* getAgent(int a_agentIdentifier);
  OrderBook* getOrderBook(int a_assetIdentifier);
  void notifyAllAgents();
  void setNextActionTime();
  Agent* getNextActor();
  double getNextActionTime();
  double calculateMarketFees(int a_price, int a_volume, OrderType a_orderType, int a_agentIdentifier);
  RandomNumberGenerator* getRNG();
  std::string getName();
  void createNewsServer(Distribution *a_newsTimeDistribution, Distribution *a_newsQualityDistribution);

 private:

  /* Name of market ; used for naming the output directory ./res/m_marketName/ */
  std::string m_marketName;
  RandomNumberGenerator *m_RNG;
  double m_currentTime;
  std::map<int, Agent*> m_agents;
  int m_agentCounter;
  std::map<int, double> m_NextActionTime;
  double m_nextActionTime;
  int m_nextActorId;
  std::map<int, OrderBook*> m_assetOrderBooks;
  int m_assetCounter;
  int m_globalOrderCounter;
  NewsServer *m_newsServer;
  double m_news;

};

#endif // __MARKET__H__
