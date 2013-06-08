#ifndef __DISTRIBUTIONGAUSSIAN__H__
#define __DISTRIBUTIONGAUSSIAN__H__

#include "Distribution.h"

class DistributionGaussian : public Distribution
{
public:
	DistributionGaussian(RandomNumberGenerator * a_rng, double a_mean = 0, double a_sigma = 1) : 
		Distribution(a_rng),m_mean(a_mean),m_sigma(a_sigma),
		m_NormalDistribution(m_rng->getRNG(),boost::normal_distribution<>(a_mean,a_sigma))
	{};
	~DistributionGaussian()
	{

	}

	double nextRandom()
	{
		return m_NormalDistribution();
	}
private:
	double m_mean;
	double m_sigma;
	NormalDistribution m_NormalDistribution;
};


#endif //__DISTRIBUTIONGAUSSIAN__H__
