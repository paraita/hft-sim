#include "Exceptions.h"
#include "Market.h"
#include "OrderBook.h"
#include "Order.h"
#include "Agent.h"
#include "NewsServer.h"
#include "RandomNumberGenerator.h"
#include <boost/filesystem.hpp>
#include <string>


Market::Market(std::string a_marketName)
{
	m_marketName = a_marketName ;
	m_agentCounter = 0;
	m_assetCounter = 0;
	m_globalOrderCounter = 0;
	m_nextActionTime = 1E300 ;
	m_nextActorId = -1 ;
	m_currentTime = 0.0;
	m_newsServer = NULL;
	m_RNG = new RandomNumberGenerator();

	// clean/create output directory
	std::string outputDirectoryName ;
//	outputDirectoryName = "./res/" + a_marketName ;
//	boost::filesystem::remove_all(outputDirectoryName.c_str() ) ;
//	boost::filesystem::create_directory(outputDirectoryName.c_str() ) ;
}

Market::~Market()
{
	if (!m_assetOrderBooks.empty())
	{
		for (std::map< int, OrderBook* >::iterator it = m_assetOrderBooks.begin(); it != m_assetOrderBooks.end(); ++it)
		{
			OrderBook* l_orderBook = it->second;
			delete l_orderBook;
		}
		m_assetOrderBooks.clear();
	}
	delete m_newsServer;
}
double Market::getCurrentTime()
{
	return m_currentTime;
}
RandomNumberGenerator * Market::getRNG()
{
	return m_RNG ;	
}

void Market::updateCurrentTime(double a_time)
{
	m_currentTime = a_time;
}
void Market::registerAgent(Agent * a_Agent)
{
	m_agents[m_agentCounter+1] = a_Agent;
	a_Agent->setIdentifier(m_agentCounter+1) ;
	for (int i = 1;i<= m_assetCounter;i++)
	{
		a_Agent->initAssetQuantity(i);
	}
	double newAgentNextActionTime = a_Agent->getNextActionTime();
	m_NextActionTime[m_agentCounter+1] = newAgentNextActionTime;

	// Check if NewAgent is the following player
	if(m_nextActionTime>newAgentNextActionTime)
	{
		m_nextActionTime = newAgentNextActionTime ;
		m_nextActorId = m_agentCounter+1 ;
	}
	m_agentCounter += 1;
}
void Market::createNewsServer(Distribution *a_newsTimeDistribution, Distribution *a_newsQualityDistribution)
{
	m_newsServer = new NewsServer(a_newsTimeDistribution,a_newsQualityDistribution);
}
void Market::setNextActionTime()
{	
	int begin = 1;
	if(m_newsServer!=NULL)
		begin = 0;

	for(int i=begin; i <= m_agentCounter; i++)// agent 0 is the news Server
	{
		if(i != m_nextActorId)
		{
			m_NextActionTime[i] -= m_NextActionTime[m_nextActorId] ;
		}
	}
	if (m_nextActorId == 0)
		m_newsServer->getNextActionTime() ;
	else
		m_NextActionTime[m_nextActorId] = m_agents[m_nextActorId]->getNextActionTime() ;

	m_nextActionTime = m_NextActionTime[begin] ;
	m_nextActorId = begin;
	for(int i=1;i <= m_agentCounter;i++)
	{
		if(m_NextActionTime[i]<m_nextActionTime)
		{
			m_nextActionTime = m_NextActionTime[i] ;
			m_nextActorId = i ;
		}
	}
	if (m_nextActorId == 0)
	{
		m_news = m_newsServer->getNews();
		setNextActionTime();
	}
}

Agent* Market::getNextActor()
{
	return m_agents[m_nextActorId];
}

double Market::getNextActionTime()
{
	return m_nextActionTime;
}
void Market::createAssets(int a_number, int a_tickSize, int a_defaultBid, int a_defaultAsk)
{
	for (int i = 1;i<= a_number;i++)
	{
		m_assetOrderBooks[m_assetCounter+i] = new OrderBook(this,m_assetCounter+i, a_tickSize,a_defaultBid,a_defaultAsk);
		for(int j=1;j<=m_agentCounter;j++)
		{
			m_agents[j]->initAssetQuantity(i);	
		}
	}
	m_assetCounter += a_number;
}
int Market::getOrderIdentifier()
{
	m_globalOrderCounter++;
	return m_globalOrderCounter;
}
Agent * Market::getAgent(int a_agentIdentifier)
{
	if( a_agentIdentifier> m_agentCounter)
		throw Exception("Agent does not exist");

	return m_agents[a_agentIdentifier];
}
OrderBook* Market::getOrderBook(int a_assetIdentifier)
{
	if( a_assetIdentifier> m_assetCounter)
		throw Exception("Asset does not exist");

	return m_assetOrderBooks[a_assetIdentifier];
}
void Market::pushOrder(Order &a_order)
{
	m_assetOrderBooks[a_order.getUnderlying()]->pushOrder(a_order);	
}
void Market::notifyExecution(int a_agentIdentifier,int a_orderIdentifier,double a_time,int a_price)
{
	double l_fee = calculateMarketFees(a_price,m_agents[a_agentIdentifier]->m_passedOrders[a_orderIdentifier].m_volume,
		m_agents[a_agentIdentifier]->m_passedOrders[a_orderIdentifier].m_type,a_agentIdentifier);
	
	m_agents[a_agentIdentifier]->chargeMarketFees(l_fee);
	m_agents[a_agentIdentifier]->notifyExecution(a_orderIdentifier,a_time,a_price);
}
void Market::notifyPartialExecution(int a_agentIdentifier,int a_orderIdentifier,double a_time,int a_volume,int a_price)
{
	double l_fee = calculateMarketFees(a_price,a_volume,
		m_agents[a_agentIdentifier]->m_passedOrders[a_orderIdentifier].m_type,a_agentIdentifier);
	
	m_agents[a_agentIdentifier]->chargeMarketFees(l_fee);
	m_agents[a_agentIdentifier]->notifyPartialExecution(a_orderIdentifier,a_time,a_volume,a_price);
}
void Market::notifyCancellation(int a_agentIdentifier,int a_orderIdentifier,double a_time)
{
	m_agents[a_agentIdentifier]->notifyCancellation(a_orderIdentifier,a_time);
}
void Market::notifyAllAgents()
{
	for(int i=1;i<=m_agentCounter;i++)
	{
		m_agents[i]->notifyAgentOfMarketEvent();
	}
}
double Market::calculateMarketFees(int a_price,int a_volume,OrderType a_orderType, int a_agentIdentifier)
{
	double l_feeRate = m_agents[a_agentIdentifier]->getFeeRate(a_orderType);
	double l_fee = a_price*a_volume*l_feeRate;
	return l_fee;
}
std::string Market::getName()
{
	return m_marketName ;
}

