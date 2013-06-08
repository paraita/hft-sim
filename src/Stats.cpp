//#include "Market.h"
//#include "OrderBook.h"
//#include "Order.h"
//#include "Stats.h"
//#include <fstream>
//#include <iostream>
//#include <cstdlib>
//
//
//Stats::Stats(OrderBook * myOrderBook)
//{
//	m_OrderBook = myOrderBook ;
//}
//
//Stats::~Stats()
//{
//	
//}
//
//std::vector<double> Stats::getPriceTimeSeries(TimeSeriesType dataType, double samplePeriod)
//{
//	std::vector<double> timeSeries ;
//	// get order book history
//	const std::vector<OrderBookHistory> fullHistory = m_OrderBook->getOrderBookHistory() ;
//	std::vector<OrderBookHistory>::const_iterator it = fullHistory.begin() ;
//	// parse history and build time series
//	int timeIndex = 0 ;
//	double timeStart = it->m_timeStamp ;
//	while(it != fullHistory.end())
//	{
//		double l_ts = it->m_timeStamp;
//		if(l_ts>=timeStart+timeIndex*samplePeriod)
//		{
//			// if consecutive identical ts, get to the last
//			double next_ts = l_ts ;
//			while(l_ts==next_ts && it != fullHistory.end()-1){
//				next_ts = (it+1)->m_timeStamp ;
//				it++ ;
//			}
//			if(l_ts!=next_ts) it-- ;
//			// now get the requested value
//			double valueToBeAdded ;
//			switch(dataType)
//			{
//				case ASK:
//					valueToBeAdded = it->m_limitOrders[0].m_ask/100.0 ;
//					break ;
//				case BID:
//					valueToBeAdded = it->m_limitOrders[0].m_bid/100.0 ;
//					break ;
//				case MID:
//					valueToBeAdded = 0.5*(it->m_limitOrders[0].m_ask/100.0 
//						+ it->m_limitOrders[0].m_bid/100.0) ;
//					break ;
//				case LAST:
//					valueToBeAdded = it->m_last ;
//					break ;
//				case SPREAD:
//					valueToBeAdded = it->m_limitOrders[0].m_ask/100.0
//						- it->m_limitOrders[0].m_bid/100.0 ;
//					break ;
//				default:
//					std::cout << "Unknown TimeSeriesType requested in Stats::getPriceTimeSeries." << std::endl ;
//					exit(1) ;
//			}
//			timeSeries.push_back(valueToBeAdded) ;
//			timeIndex++ ;
//		}
//		it++ ;
//	}
//	return timeSeries ;
//}
//
//void Stats::printTimeSeries(TimeSeriesType dataType, double samplePeriod)
//{
//	// get timeseries
//	std::vector<double> timeSeries = getPriceTimeSeries(dataType, samplePeriod);
//	// prepare outputFile
//	std::ostringstream ss_fileName ;
//	ss_fileName << "./res/" << m_OrderBook->getLinkToMarket()->getName() << "/" ;
//	switch(dataType)
//	{
//	case ASK:
//		ss_fileName << "Ask_" ;
//		break ;
//	case BID:
//		ss_fileName << "Bid_" ;
//		break ;
//	case MID:
//		ss_fileName << "Mid_" ;
//		break ;
//	case LAST:
//		ss_fileName << "Last_" ;
//		break ;
//	default:
//		std::cout << "Unknown TimeSeriesType requested in Stats::printTimeSeries." << std::endl ;
//		exit(1) ;
//	}
//	ss_fileName << samplePeriod << "_sampledTimeSeries.dat" ;
//	// Print time series
//	std::fstream outputFile(ss_fileName.str().c_str()) ;
//	for(unsigned int idx=0; idx<timeSeries.size(); idx++){
//		outputFile<<timeSeries[idx]<<"\n";
//	}
//	outputFile.close() ;
//}
//
////void Stats::plotPDF(const std::string dataName, const std::vector<double> data, int nBins)
////{
////	long nElements = data.size() ;
////	double * dataArray = (double*)malloc(nElements*sizeof(double));
////	for(int k=0;k<nElements;k++){
////		dataArray[k] = data[k] ;	
////	}
////	// Estimate number of bins
////	if(nBins==-1){
////		nBins = 20; //nElements/100 ;
////	}
////	// Allocate memory
////	gsl_histogram * dataHistogram = gsl_histogram_alloc (nBins) ;
////	gsl_histogram_set_ranges_uniform (dataHistogram, 
////		gsl_stats_min(dataArray, 1, nElements),
////		gsl_stats_max(dataArray, 1, nElements)
////	) ;
////	// Build histogram
////	for(int k=0;k<nElements;k++){
////		gsl_histogram_increment (dataHistogram, dataArray[k]) ;
////	}
////	// Normalize as a pdf
////	gsl_histogram_pdf * pdfHist = gsl_histogram_pdf_alloc (nBins) ;
////	gsl_histogram_pdf_init (pdfHist, dataHistogram) ;
////	// Print
////	FILE * outFile;
////	std::ostringstream outFilename ;
////	outFilename << "./res/" << m_OrderBook->getLinkToMarket()->getName() << "/" << dataName << "_PDF.csv" ;
////	outFile=fopen(outFilename.str().c_str(),"w");
////	for(int i=0;i<nBins;i++)
////	{
////		fprintf(outFile, "%f\t%f\n", 0.5*(pdfHist->range[i]+pdfHist->range[i+1]),
////			pdfHist->sum[i+1]-pdfHist->sum[i]);
////	}
////	fclose(outFile);
////}
////
////void Stats::plotNormalizedPDF(const std::string dataName, const std::vector<double> data, int nBins)
////{
////	// Normalize elements (center and reduce)
////	long nElements = data.size() ;
////	double * dataArray = (double*)malloc(nElements*sizeof(double));
////	for(int k=0;k<nElements;k++){
////		dataArray[k] = data[k] ;	
////	}
////	double m1 = gsl_stats_mean (dataArray, 1, nElements) ;
////	double sd = gsl_stats_sd (dataArray, 1, nElements) ;
////	std::cout << dataName << "\t" << m1 << "\t" << sd << std::endl ;
////	std::vector<double> normalizedData ;
////	for(int k=0;k<nElements;k++){
////		normalizedData.push_back((data[k]-m1)/sd) ;	
////	}
////	// Allocate memory
////	nBins = 24 ;
////	double lowerBound = -6 ;
////	double upperBound = 6 ;
////	gsl_histogram * dataHistogram = gsl_histogram_alloc (nBins) ;
////	gsl_histogram_set_ranges_uniform (dataHistogram, 
////		lowerBound,
////		upperBound
////	) ;
////	// Build histogram
////	for(int k=0;k<nElements;k++){
////		gsl_histogram_increment (dataHistogram, normalizedData[k]) ;
////	}
////	// Normalize as a pdf
////	gsl_histogram_pdf * pdfHist = gsl_histogram_pdf_alloc (nBins) ;
////	gsl_histogram_pdf_init (pdfHist, dataHistogram) ;
////	// Print
////	FILE * outFile;
////	std::ostringstream outFilename ;
////	outFilename << "./res/" << m_OrderBook->getLinkToMarket()->getName() << "/" << dataName << "_NormalizedPDF.csv" ;
////	outFile=fopen(outFilename.str().c_str(),"w");
////	for(int i=0;i<nBins;i++)
////	{
////		fprintf(outFile, "%f\t%f\t%f\t%f\n", pdfHist->range[i], pdfHist->range[i+1],
////			0.5*(pdfHist->range[i]+pdfHist->range[i+1]),
////			pdfHist->sum[i+1]-pdfHist->sum[i]);
////	}
////	fclose(outFile);
////}
////
//std::vector<double> Stats::getOrderSignsTimeSeries(TimeSeriesType dataType)
//{
//	std::vector<double> timeSeries ;
//	// get order book history
//	std::vector<Order> fullOrderHistory = m_OrderBook->getOrderHistory() ;
//	std::vector<Order>::iterator it = fullOrderHistory.begin() ;
//	// parse history and build time series
//	while(it != fullOrderHistory.end())
//	{
//		// Is this order to be considered ?
//		bool toBeConsidered = false ;
//		switch(dataType)
//		{
//			case MARKET:
//				if(it->getType()==MARKET_SELL || it->getType()==MARKET_BUY) toBeConsidered = true ;
//				break ;
//			case LIMIT:
//				if(it->getType()==LIMIT_SELL || it->getType()==LIMIT_BUY) toBeConsidered = true ;
//				break ;
//			case ALL:
//				toBeConsidered = true ;
//				break ;
//			default:
//				std::cout << "Unknown TimeSeriesType requested in Stats::getOrderSignsTimeSeries." << std::endl ;
//				exit(1) ;
//		}
//		// get the requested value
//		if(toBeConsidered)
//		{
//			int valueToBeAdded ;
//			if(it->getType()==MARKET_SELL || it->getType()==LIMIT_SELL) valueToBeAdded = -1 ;
//			if(it->getType()==MARKET_BUY || it->getType()==LIMIT_BUY) valueToBeAdded = 1 ;
//			timeSeries.push_back(valueToBeAdded) ;
//		}
//		it++ ;
//	}
//	return timeSeries ;
//}
//
//std::map<int,double> Stats::getAutocorrelation(std::vector<double> data, int lagMax, int deltaLag)
//{
//	std::map<int,double> result ;
//	int lag = 0 ;
//	double norm = 1.0 ;
//	while (lag<=lagMax) {
//		double res = 0.0 ;
//		for(unsigned int i=lag; i<data.size(); i++)
//		{
//			res += 1.0*(data[i] * data[i-lag])/(data.size()-lag) ;
//		}
//		result[lag] = res;
//		if(lag==0) norm = res ;
//		result[lag] /= norm ;
//		lag += deltaLag ;
//	}
//	return result ;
//}
//
//void Stats::printAutocorrelation(std::string tag, std::vector<double> data, int lagMax, int deltaLag)
//{
//	// get time series
//	std::map<int,double> autocorrelations = getAutocorrelation(data, lagMax, deltaLag) ;
//	// prepare outputFile
//	std::ostringstream ss_fileName ;
//	ss_fileName << "./res/" << m_OrderBook->getLinkToMarket()->getName() << "/Autocorrelation_" << tag << ".csv" ;
//	// Print time series
//	std::fstream outputFile(ss_fileName.str().c_str()) ;
//	std::map<int,double>::iterator it = autocorrelations.begin() ;
//	while(it!=autocorrelations.end()){
//		outputFile<<(it->first, "%d")<<'\t'<<(it->second,"%e")<<'\n' ;
//		it++ ;
//	}
//	outputFile.close() ;
//}
//
//void Stats::printSummary()
//{
//	// prepare outputFile
//	std::ostringstream ss_fileName ;
//	ss_fileName << "./res/" << m_OrderBook->getLinkToMarket()->getName() << "/Summary.csv" ;
//	std::fstream outputFile(ss_fileName.str().c_str()) ;
//	// get order history
//	std::vector<Order> orderHistory = m_OrderBook->getOrderHistory() ;
//	int nBuyLimitOrders = 0 ;
//	int nSellLimitOrders = 0 ;
//	int nBuyMarketOrders = 0 ;
//	int nSellMarketOrders = 0 ;
//	int totalVolumeExchanged = 0 ;
//	std::vector<Order>::iterator iter = orderHistory.begin() ;
//	while(iter!=orderHistory.end())
//	{
//		// count orders by type
//		if(iter->getType()==MARKET_BUY) nBuyMarketOrders++ ;
//		if(iter->getType()==MARKET_SELL) nSellMarketOrders++ ;
//		if(iter->getType()==LIMIT_BUY) nBuyLimitOrders++ ;
//		if(iter->getType()==LIMIT_SELL) nSellLimitOrders++ ;
//		// count volume exchanged
//		if(iter->getType()==MARKET_BUY || iter->getType()==MARKET_SELL)
//		{
//			totalVolumeExchanged += iter->getInitialVolume() ;
//		}
//		// next order
//		iter++;
//	}
//	// write output
//	outputFile << "nBuyLimitOrders\t" << nBuyLimitOrders << '\n';
//	outputFile<< "nSellLimitOrders\t"<< nSellLimitOrders <<'\n';
//	outputFile<< "nBuyMarketOrders\t"<< nBuyMarketOrders <<'\n';
//	outputFile<< "nSellMarketOrders\t"<< nSellMarketOrders <<'\n';
//	outputFile<< "totalVolumeExchanged\t"<< totalVolumeExchanged <<'\n';
//	// close output file
//	outputFile.close() ;
//}
//
//
//
//
