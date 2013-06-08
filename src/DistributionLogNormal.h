#ifndef __DISTRIBUTIONLOGNORMAL__H__
#define __DISTRIBUTIONLOGNORMAL__H__



#include "Distribution.h"

class DistributionLogNormal : public Distribution
{
	public:
		DistributionLogNormal(RandomNumberGenerator * a_rng, double a_logmean = 0, double a_logsigma = 1) : 
			Distribution(a_rng),m_mean(a_logmean),m_sigma(a_logsigma),
			m_LogNormalDistribution(m_rng->getRNG(),boost::lognormal_distribution<>(a_mean,a_sigma))
		{};
		~DistributionLogNormal()
		{
		
		}

		double nextRandom()
		{
			return m_LogNormalDistribution();
		}
	private:
		double m_mean;
		double m_sigma;
		LogNormalDistribution m_LogNormalDistribution;
};

#endif //__DISTRIBUTIONLOGNORMAL__H__
