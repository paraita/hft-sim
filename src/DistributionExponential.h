#ifndef __DISTRIBUTIONEXPONENTIAL__H__
#define __DISTRIBUTIONEXPONENTIAL__H__

#include <cmath>
#include <iostream>


#include "Distribution.h"

class DistributionExponential : public Distribution
{
	public:
		DistributionExponential(RandomNumberGenerator * a_rng, double a_lambda = 1.0) :
			Distribution(a_rng),m_lambda(a_lambda),
			m_ExponentialDistribution(a_rng->getRNG(),boost::exponential_distribution<>(1.0/a_lambda ))
			// To have the lambda be the mean of the exponential distribution
		 {};

		~DistributionExponential()
		{
		
		}
	
		double nextRandom()
		{
			return m_ExponentialDistribution();
		}
	private:
		double m_lambda ; // = mean
		ExponentialDistribution m_ExponentialDistribution;
};

#endif //__DISTRIBUTIONEXPONENTIAL__H__
