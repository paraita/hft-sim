#ifndef __RANDOMNUMBERGENERATOR__H__
#define __RANDOMNUMBERGENERATOR__H__

#include <boost/random.hpp>

typedef boost::mt19937 GeneratorType;

typedef boost::variate_generator< GeneratorType&, boost::exponential_distribution<> > ExponentialDistribution;
typedef boost::variate_generator< GeneratorType&, boost::normal_distribution<> > NormalDistribution;
typedef boost::variate_generator< GeneratorType&, boost::lognormal_distribution<> > LogNormalDistribution;
typedef boost::variate_generator< GeneratorType&, boost::gamma_distribution<> > GammaDistribution;

class RandomNumberGenerator
{
	public :
		RandomNumberGenerator()
		{
			m_rng=GeneratorType();
			m_rng.seed(static_cast<unsigned int>(std::time(0)));
		};
		~RandomNumberGenerator(){};
		GeneratorType& getRNG()
		{
			return m_rng;
		}
	private:
		GeneratorType m_rng;
};
#endif