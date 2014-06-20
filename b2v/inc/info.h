#ifndef __INFO_H__
#define __INFO_H__

#include <string>
#include "util.h"

class Info
{
	public:
		std::string inputFileName;
		std::string outFileName;
		std::string color;
		pixel bgColor;
		double toleranceCurve;
		double toleranceLine;
		bool bgColorProvided;
		bool switchNoisy;
		int turdSize;
		int medianBlurKernelSize;
		int KMeansMaxIters;
		int numClusters;

		Info();
		~Info();
		void parseInputArg(int, char**);
};

#endif