/* Copyright (C) 2014 Rembrandt Bakker, Dhruv Kohli, Piotr Majka
   This file is part of mindthegap. It is free software and it is covered
   by the GNU General Public License. See the file LICENSE for details. */
   
//main.cpp
/*
 * Google Summer Of Code
 * Project: Real-time vectorization of brain-atlases
 * Mentoring Organization: International Neuroinformatics Coordinating Facility
 * Mentors: Rembrandt Bakker, Piotr Majka
 * Student: Dhruv Kohli
 * blog: https://rtvba.weebly.com
 * Software name: mindthegap
 * Brief: Vectorize bitmaps without introducing gap/overlaps between adjacent areas
 */

/*
 * License info
 */

#include "info.h"
#include "bitmap.h"

int main(int argc, char **argv)
{
	Info inputInfo;

	//parse arguments to main
	inputInfo.parseInputArg(argc, argv);

	//create bitmap with info provided provided by the user
	Bitmap inputBitmap(&inputInfo);

	//process and write output
	inputBitmap.processImage();
	inputBitmap.writeOuputSVG();
	
	return 0;
}