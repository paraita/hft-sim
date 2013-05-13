#include "Agent.h"
#include "Market.h"
#include "Order.h"
#include "OrderBook.h"
#include "DistributionUniform.h"
#include "DistributionGaussian.h"
#include "DistributionExponential.h"
#include "DistributionConstant.h"
#include "LiquidityProvider.h"
#include "NoiseTrader.h"
#include "MarketMaker.h"
#include "Exceptions.h"
#include "Plot.h"
#include "Stats.h"
#include "RandomNumberGenerator.h"


int main(int argc, char const *argv[])
{
	RandomNumberGenerator * myRNG = new RandomNumberGenerator();

	// LiquidityProvider
	DistributionExponential *LimitOrderActionTimeDistribution = new DistributionExponential(myRNG, 0.35);
	DistributionGaussian *LimitOrderOrderVolumeDistribution = new DistributionGaussian(myRNG, 0.7*100, sqrt(0.2 * 100));
	DistributionExponential *LimitOrderOrderPriceDistribution = new DistributionExponential(myRNG, 6);

	// TODO continuer ici
	LiquidityProvider *myLiquidityProvider = new LiquidityProvider(myMarket,
			LimitOrderActionTimeDistribution,
			LimitOrderOrderVolumeDistribution,
			LimitOrderOrderPriceDistribution,
			buyFrequencyLP,
			1,
			cancelBuyFrequencyLP,
			cancelSellFrequencyLP,
			uniformCancellationProbability);

	std::cout << "titoi" << std::endl;

	delete myRNG;
	return 0;
}