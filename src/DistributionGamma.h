#ifndef __DISTRIBUTIONGAMMA__H__
#define __DISTRIBUTIONGAMMA__H__

#include "Distribution.h"




class DistributionGamma : public Distribution
{
	public:
		DistributionGamma(RandomNumberGenerator * a_rng, double a_shape = 1.0, double a_scale = 1.0) : 
			Distribution(a_rng),m_shape(a_shape),m_scale(a_scale),
			m_GammaDistribution(a_rng->getRNG(),boost::gamma_distribution<>(a_shape,a_scale))
		{};
		~DistributionGamma()
		{
		
		}
	
		double nextRandom()
		{
			return m_GammaDistribution();
		}
	private:
		double m_shape;
		double m_scale;
		GammaDistribution m_GammaDistribution;
};

#endif //__DISTRIBUTIONGAMMA__H__
