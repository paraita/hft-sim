#include <fstream>
#include <iostream>
#include <cstdio>
#include "Plot.h"

Plot::Plot() {
	m_gnuPlot = popen("gnuplot", "w");
}

Plot::~Plot() { }

void Plot::plot() {
	std::ofstream outFile("graphe.data");
	for(int i = 0; i < 100; i++) {
		outFile << (double) i << '\t' << (double) i << std::endl;
	}
	outFile.close();
	fflush (m_gnuPlot);
	fprintf(m_gnuPlot, "set boxwidth 0.9 relative\n");
	fprintf(m_gnuPlot, "set style fill 1.0 \n");
	fprintf(m_gnuPlot, "plot \"graphe.data\" with boxes\n");
	fflush (m_gnuPlot);
}

void Plot::plot(std::string dataName,
		int size,
		const double *x,
		const double *y) {
	std::ofstream outFile("graphe.data");
	for(int i = 0; i < size; i++) {
		outFile << x[i] << '\t' << y[i] << std::endl;
	}
	outFile.close();
	fflush (m_gnuPlot);
	std::ostringstream oss_command ;
	oss_command << "plot \'graphe.data\' title \' " << dataName << "\' with lines" << std::endl;
	fprintf(m_gnuPlot, "%s", oss_command.str().c_str());
	fflush (m_gnuPlot);
}


void Plot::plotPrices(const std::vector<double>& x, const std::vector<int>& y) {
	std::ofstream outFile("Prices.data");
	for(unsigned int i = 0; i < x.size(); i++) {
		outFile << x[i] << '\t' << y[i] / 100.0 << std::endl;
	}
	outFile.close();

	fflush (m_gnuPlot);

	fprintf(m_gnuPlot, "set xr [*:*]\n");
	fprintf(m_gnuPlot, "set yr [*:*]\n");
	fprintf(m_gnuPlot, "set title \"History of Prices \"\n");
	fprintf(m_gnuPlot,"plot \"Prices.data\" using 1:2 with lines \n");
	fflush (m_gnuPlot);
}
//-----------------------------------------------------------------------------
void Plot::plotOrderBook(const std::vector<int>& x, 
		const std::vector<int>& y,
		int last,
		double volatility,
		std::vector<int>& MMprices,
		std::vector<int> & MMvolumes) {
	std::ofstream outFile("OrderBook.data");
	std::ofstream outFileMM("MMdata.data");
	int index(0);
	std::vector<int> x2;
	std::vector<int> y2;
	x2.push_back(x[0]);
	y2.push_back(y[0]);
	int pold(x[0]);
	for(unsigned int i = 1; i < x.size(); i++) {
		if(x[i] != pold + 1) {
			x2.push_back(pold + 1);
			y2.push_back(0);
			pold = pold + 1;
			i--;
		}
		else {
			x2.push_back(x[i]);
			y2.push_back(y[i]);
			pold = x[i];
		}
	}
	while((y2[index] - y2[index+1]) >= y2[index]) {
		index++;
	}
	pold = x2[std::max(index-20, 0)];
	for(unsigned int i = std::max(index - 20 + 1, 1); i < std::min(index + 21, (int) x2.size()); i++) {
		if(x2[i] != pold + 1) {
			outFile << (pold + 1) / 100.0 << '\t' << 0 << std::endl;
			pold = pold + 1;
			i--;
		}
		else {
			outFile << x2[i] / 100.0 << '\t' << y2[i] << std::endl;
			pold = x2[i];
		}
	}

	outFile.close();

	for(unsigned int k = 0; k < MMprices.size(); k++) {
		outFileMM << MMprices[k] / 100.0 << '\t'<< MMvolumes[k] << std::endl;
	}
	outFileMM.close();

	fflush (m_gnuPlot);
	fprintf(m_gnuPlot, "set boxwidth 0.7 relative\n");
	fprintf(m_gnuPlot, "set grid ytics noxtics\n");
	fprintf(m_gnuPlot, "set grid no ytics\n");
	fprintf(m_gnuPlot, "set tic front\n");
	fprintf(m_gnuPlot, "set xr [99.6:100.4]\n");
	fprintf(m_gnuPlot, "set yr [-250:250]\n");
	fprintf(m_gnuPlot, "set style fill solid 2.0 \n");
	fprintf(m_gnuPlot, "set title \"Last Price = %f, Volatility = %f\"\n",  (double) (last/100.0), volatility);
	fprintf(m_gnuPlot, "set zeroaxis\n");
	//fprintf(m_gnuPlot, "set xtic rotate by 45 font \",7\"");
	fprintf(m_gnuPlot, "set xtics offset 0,-0.8,0 axis 0.01 rotate by 80\n");
	fprintf(m_gnuPlot, "set xtics front");
	fprintf(m_gnuPlot, "show xtics\n");
	fprintf(m_gnuPlot,"plot \"OrderBook.data\"  using 1:($2 < 0? $2 : 1/0 ) lc rgb \"blue\" title \"Bid (Achat)\"with boxes,\'\' using 1:($2 > 0 ? $2 : 1/0 ) lc rgb \"red\"  title \"Ask (Vente)\" with boxes ,  \"MMdata.data\" using 1:2 lc rgb \"green\" with boxes \n");
	//fprintf(m_gnuPlot,"plot \"OrderBook.data\"  using 1:($2 < 0? $2 : 1/0 ) lc rgb \"blue\" title \"Bid (Achat)\"with boxes,\'\' using 1:($2 > 0 ? $2 : 1/0 ) lc rgb \"red\"  title \"Ask (Vente)\" with boxes , \"MMdata.data\" using 1:2 ti \"Slower\" linecolor rgb \"#F76258\" title \"Market Marker orders\" with boxes \n");
	//fprintf(m_gnuPlot,"plot \"OrderBook.data\"  using 1:2 lc rgb \"blue\" with boxes, \"MMdata.data\" using 1:2 lc rgb \"green\" with boxes \n");
	fflush (m_gnuPlot);
}

void Plot::plot2OrderBooks(const std::vector<int>& x1,
		const std::vector<int>& y1,
		int last1,
		const std::vector<int>& x2,
		const std::vector<int>& y2,
		int last2) {
	std::ofstream outFile1("OrderBook1.data");
	for(unsigned int i = 0; i < x1.size(); i++) {
		outFile1 << x1[i] / 100.0 << '\t' << y1[i] << std::endl;
	}
	outFile1.close();

	std::ofstream outFile2("OrderBook2.data");
	for(unsigned int i = 0; i < x2.size(); i++) {
		outFile2 << x2[i] / 100.0 << '\t' << y2[i] << std::endl;
	}
	outFile2.close();

	fflush (m_gnuPlot);
	fprintf(m_gnuPlot, "set size 1.0, 1.0\n");
	fprintf(m_gnuPlot, "set origin 0.0, 0.0\n");
	fprintf(m_gnuPlot, "set multiplot\n");
	fprintf(m_gnuPlot, "set size 1.0,0.5\n");
	fprintf(m_gnuPlot, "set origin 0.0, 0.5\n");
	fprintf(m_gnuPlot, "set title \"Last Price = %f\"\n",  (double)last1/100.0);
	fprintf(m_gnuPlot, "plot \"OrderBook1.data\" with boxes\n");
	fprintf(m_gnuPlot, "set size 1.0,0.5\n");
	fprintf(m_gnuPlot, "set origin 0.0, 0.0\n");
	fprintf(m_gnuPlot, "set title \"Last Price = %f\"\n",  (double)last2/100.0);
	fprintf(m_gnuPlot, "plot \"OrderBook2.data\" with boxes\n");
	fflush (m_gnuPlot);
}
