#ifndef __NEWSSERVER__H__
#define __NEWSSERVER__H__

class Distribution;
class NewsServer
{
	public :
		NewsServer(Distribution *a_newsTimeDistribution,Distribution * a_newsQualityDistribution);
		virtual ~NewsServer();
		double getNextActionTime() const;
		double getNews() const;
	
	private:
		Distribution *m_newsTimeDistribution;
		Distribution *m_newsQualityDistribution;


};
#endif