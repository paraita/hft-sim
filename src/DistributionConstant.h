#ifndef __DISTRIBUTIONCONSTANT__H__
#define __DISTRIBUTIONCONSTANT__H__

#include "Distribution.h"

class DistributionConstant : public Distribution
{
	public:
		DistributionConstant(RandomNumberGenerator * a_rng, double a_Constant = 0) : Distribution(a_rng)
		{
			m_Constant = a_Constant;
		}
		~DistributionConstant()
		{
		
		}

		double nextRandom()
		{
			return m_Constant;
		}
	private:
		double m_Constant ;
};

#endif /*__DISTRIBUTIONCONSTANT__H__*/
