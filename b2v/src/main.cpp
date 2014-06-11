/*
 * Google Summer Of Code
 * Project: Real-time vectorization of brain-atlases
 * Mentoring Organization: International Neuroinformatics Coordinating Facility
 * Mentors: Rembrandt Bakker, Piotr Majka
 * Student: Dhruv Kohli
 * blog: https://rtvba.weebly.com
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include "bitmap.h"

int main(int argc, char **argv)
{
	std::string filename = "";
	std::string outFileName = "";
	uint tolerance = 0;


	//parsing arguments
	int i = argc;
	while(i > 1)
	{
		std::string temp(argv[i - 1]);
		if(temp == "-i")
		{
			if(argc != i)
				filename = argv[i];
			else
			{
				std::cout << "invalid input file" << std::endl;
				exit(1);
			}
		}
		else if(temp == "-o")
		{
			if(argc != i)
				outFileName = argv[i];
			else
			{
				std::cout << "invalid input file" << std::endl;
				exit(1);
			}
		}
		else if(temp == "-t")
		{
			if(argc != i)
				tolerance = atoi(argv[i]);
			else
			{
				std::cout << "invalid tolerance" << std::endl;
				exit(1);
			}
		}
		else if(temp == "-h")
		{
			std::cout << "b2v: Transforms bitmaps to vector graphics\n\nusage: " << ROOT_DIR << "/bin/b2v [-h] -i BITMAP_SRC [-o SVG_DEST] [-t FIT_TOLERANCE]\n\ndescription\n\narguments: \n-h, --help \n\t\t show this help message and exit\n-i BITMAP_SRC \n\t\t Path to input PNG Bitmap image\n-o SVG_DEST \n\t\t Destination of output SVG with desired name of file(default=ouput.svg)\n-t FIT_TOLERANCE \n\t\t Fitting tolerance(default=0)\n" << std::endl;
			exit(1);
		}
		i--;
	}

	if(tolerance < 0)
	{
		std::cout << "invalid tolerance: must be greater than zero" << std::endl;
		exit(1);
	}

	if(outFileName == "")
	{
		outFileName = "ouput.svg";
	}

	if(filename == "")
	{
		std::cout << "input file not provided: use -i filename.png" << std::endl;
		exit(1);
	}

	Bitmap *inputBitmap = new Bitmap(filename);
	inputBitmap->processImage(tolerance);
	inputBitmap->writeOuputSVG(outFileName);
	
	return 0;
}

