#include "Distribution.h"
#include "NewsServer.h"

NewsServer::NewsServer(Distribution *a_newsTimeDistribution, Distribution *a_newsQualityDistribution)
{
	m_newsTimeDistribution = a_newsTimeDistribution;
	m_newsQualityDistribution = a_newsQualityDistribution;
}
NewsServer::~NewsServer()
{

}
double NewsServer::getNextActionTime() const
{
	return m_newsTimeDistribution->nextRandom() ;
}
double NewsServer::getNews() const
{
	return m_newsQualityDistribution->nextRandom() ;
}