#ifndef __DISTRIBUTIONUNIFORM__H__
#define __DISTRIBUTIONUNIFORM__H__

#include <boost/random.hpp>
#include "RandomNumberGenerator.h"
#include "Distribution.h"

class DistributionUniform : public Distribution
{
	public:
		DistributionUniform(RandomNumberGenerator * a_rng, double a_lowerBound = 0, double a_upperBound = 1) : Distribution(a_rng)
		{
			m_lowerBound = a_lowerBound;
			m_upperBound = a_upperBound;
		}
		~DistributionUniform()
		{
		
		}

		double nextRandom()
		{
			static boost::uniform_01<GeneratorType> unif01(m_rng->getRNG()); 
			//see http://www.bnikolic.co.uk/blog/cpp-boost-uniform01.html for the mystery of static
			return m_lowerBound+unif01()*(m_upperBound-m_lowerBound);
		}
	private:
		double m_lowerBound ;
		double m_upperBound ;
};

#endif //__DISTRIBUTIONUNIFORM__H__
