#ifndef __DISTRIBUTION__H__
#define __DISTRIBUTION__H__

#include "RandomNumberGenerator.h"

class Distribution 
{
	public:
		Distribution(RandomNumberGenerator * a_rng)
		{
			m_rng = a_rng;
		}
		virtual ~Distribution()
		{
		
		}
		
		virtual double nextRandom() = 0 ;
		
	protected:
		RandomNumberGenerator * m_rng;
	
};


#endif //__DISTRIBUTION__H__
