#ifndef __STATS__H__
#define __STATS__H__

class Market;
class OrderBook;
#include "Types.h"
#include <vector>
#include <sstream>
#include <map>

typedef enum
{
	ASK,
	BID,
	MID,
	SPREAD,
	LAST,
	MARKET,
	LIMIT,
	ALL
} TimeSeriesType ;


class Stats
{
	public :
		Stats(OrderBook * myOrderBook);
		virtual ~Stats();

		std::vector<double> getPriceTimeSeries(TimeSeriesType dataType, double samplePeriod);
		std::vector<double> getOrderSignsTimeSeries(TimeSeriesType dataType);
		void printTimeSeries(TimeSeriesType dataType, double samplePeriod);
		void printAutocorrelation(std::string tag, std::vector<double> data, int lagMax, int deltaLag);
		void plotPDF(const std::string name, const std::vector<double> data, int nBins = -1) {};
		void plotNormalizedPDF(const std::string name, const std::vector<double> data, int nBins = -1) {};

		void printSummary() ;

	private :
		OrderBook * m_OrderBook ;
		std::map<int,double> getAutocorrelation(std::vector<double> data, int lagMax, int deltaLag) ;

};


#endif /*STATS_H_*/
