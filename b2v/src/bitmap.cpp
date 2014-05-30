#include "bitmap.h"
#include "fstream"
#include "iostream"
#include "debug.h"


// Constructor
Bitmap::Bitmap(std::string filename)
{
	uint h, w;
	std::vector<uchar> v;	//the raww pixel

	decodeOneStep(filename.c_str(), h, w, v);
	//the pixels are now in the vector "v", 4 bytes per pixel, ordered RGBARGBA

	//Initialization and allocation of private variables
	intialize(&orig, h, w);
	intialize(&pre, h + 2, w + 2);
	intialize(&pop, 2 * (h + 2), 2 * (w + 2));
	pixToNodeMap = new int*[2 * (h + 2)];
	for(uint i = 0; i < 2 * (h + 2); ++i)
	{
		pixToNodeMap[i] = new int[2 * (w + 2)];
	}
	numControlPoints = 0;

	#ifdef _TEST_1_
	//file to store r g b values of each pixel in row major order under test 1
	std::ofstream ofsTest1("testing/test_1_cpp.txt", std::ofstream::out);
	ofsTest1 << h << std::endl;
	ofsTest1 << w << std::endl;
	#endif

	//Assigning values to the pixels of original image matrix
	for(uint i = 0; i < h; ++i)
	{
		for(uint j = 0; j < w; ++j)
		{
			orig->pixMap[i][j].r = v[4 * (i * w + j)];
			orig->pixMap[i][j].g = v[4 * (i * w + j) + 1];
			orig->pixMap[i][j].b = v[4 * (i * w + j) + 2];

			#ifdef _TEST_1_
			//print r g b int values in new lines
			ofsTest1 << (uint)v[4 * (i * w + j)] << std::endl; 
			ofsTest1 << (uint)v[4 * (i * w + j) + 1] << std::endl;
			ofsTest1 << (uint)v[4 * (i * w + j) + 2] << std::endl;
			#endif
		}
	}

	#ifdef _TEST_1_
	//close test_1_cpp.txt file
	ofsTest1.close();
	#endif	
}

void Bitmap::processImage()
{
	preprocess();
	popoutBoundaries();
	codedImage = new CodeImage(pop);
	graph = new Graph(pop->height, pop->width, *codedImage->getColCode());
	detectControlPoints();
	formAdjacencyList();
	graph->formLineSegments();
	graph->preprocessLineSegments();
	graph->assignCurveNumToRegion();
	graph->formCurves();
	graph->processRegions();
}

void Bitmap::writeOuputSVG(std::string outFileName)
{
	graph->writeOuput(outFileName);
}

//Destructor
Bitmap::~Bitmap()
{
	//delete pixToNodeMap
	for(uint i = 0; i < pop->height; ++i)
	{
		delete[] pixToNodeMap[i];
	}
	delete[] pixToNodeMap;

	//delete orig
	del(orig);

	//delete pre
	del(pre);

	//delete pop
	del(pop);

	//delete coded image
	delete[] codedImage;

	//delete graph
	delete[] graph;
}

//deletes an image matrix
void Bitmap::del(ImageMatrix *a)
{
	for(uint i = 0; i < a->height; ++i)
	{
		delete[] a->pixMap[i];
	}
	delete[] a->pixMap;
	delete a;
}

//Decode from disk to raw pixels with a single function call
void Bitmap::decodeOneStep(const char* filename, uint &height, uint &width, std::vector<uchar> &image)
{
  //decode
  unsigned error = lodepng::decode(image, width, height, filename);

  //if there's an error, display it
  if(error) 
  	std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

//Initialize image matrix
void Bitmap::intialize(ImageMatrix **m, uint h, uint w)
{
	*m = new ImageMatrix;
	(*m)->height = h;
	(*m)->width = w;
	(*m)->pixMap = new pixel*[h];
	for(uint i = 0; i < h; ++i)
	{
		(*m)->pixMap[i] = new pixel[w];
	}
}

/*
Appends row at top and bottom and col at left and right of 
orig image of black (0,0,0) pixels
*/
void Bitmap::preprocess()
{
	uint h = pre->height;
	uint w = pre->width;

	for(uint i = 0; i < h; ++i)
	{
		for(uint j = 0; j < w; ++j)
		{
			if(i == 0 || j == 0 || i == h - 1 || j == w - 1)
			{
				pre->pixMap[i][j].r = pre->pixMap[i][j].g = pre->pixMap[i][j].b = 0;
			}
			else
			{
				pre->pixMap[i][j].r = orig->pixMap[i - 1][j - 1].r;
				pre->pixMap[i][j].g = orig->pixMap[i - 1][j - 1].g;
				pre->pixMap[i][j].b = orig->pixMap[i - 1][j - 1].b;
			}
		}
	}
}

void Bitmap::popoutBoundaries()
{
	uint h = pop->height;
	uint w = pop->width;
	
	for(uint i = 0; i < h; ++i)
	{
		for(uint j = 0; j < w; ++j)
		{
			pop->pixMap[i][j].r = pop->pixMap[i][j].g = pop->pixMap[i][j].b = 0;
		}
	}

	for(uint i = 1; i < h * 0.5; ++i)
	{
		uint focus_i = 2 * i + 1;
		for(uint j = 1; j < w * 0.5; ++j) 
		{
			uint focus_j = 2 * j + 1;

			pop->pixMap[focus_i][focus_j].r = pre->pixMap[i][j].r;
			pop->pixMap[focus_i][focus_j].g = pre->pixMap[i][j].g;
			pop->pixMap[focus_i][focus_j].b = pre->pixMap[i][j].b;

			if(!ifEqualPixel(pre->pixMap[i][j], pre->pixMap[i][j - 1]))
			{
				pop->pixMap[focus_i][focus_j - 1].r = pop->pixMap[focus_i][focus_j - 1].g = pop->pixMap[focus_i][focus_j - 1].b = 255;
			}
			else
			{
				pop->pixMap[focus_i][focus_j - 1].r = pre->pixMap[i][j - 1].r;
				pop->pixMap[focus_i][focus_j - 1].g = pre->pixMap[i][j - 1].g;
				pop->pixMap[focus_i][focus_j - 1].b = pre->pixMap[i][j - 1].b;
			}
			if(!ifEqualPixel(pre->pixMap[i][j], pre->pixMap[i - 1][j - 1]))
			{
				pop->pixMap[focus_i - 1][focus_j - 1].r = pop->pixMap[focus_i - 1][focus_j - 1].g = pop->pixMap[focus_i - 1][focus_j - 1].b = 255;
			}
			else
			{
				pop->pixMap[focus_i - 1][focus_j - 1].r = pre->pixMap[i - 1][j - 1].r;
				pop->pixMap[focus_i - 1][focus_j - 1].g = pre->pixMap[i - 1][j - 1].g;
				pop->pixMap[focus_i - 1][focus_j - 1].b = pre->pixMap[i - 1][j - 1].b;
			}
			if(!ifEqualPixel(pre->pixMap[i][j], pre->pixMap[i - 1][j]))
			{
				pop->pixMap[focus_i - 1][focus_j].r = pop->pixMap[focus_i - 1][focus_j].g = pop->pixMap[focus_i - 1][focus_j].b = 255;
			}
			else
			{
				pop->pixMap[focus_i - 1][focus_j].r = pre->pixMap[i - 1][j].r;
				pop->pixMap[focus_i - 1][focus_j].g = pre->pixMap[i - 1][j].g;
				pop->pixMap[focus_i - 1][focus_j].b = pre->pixMap[i - 1][j].b;
			}
		}
	}

	#ifdef _TEST_2_
	//file to store r g b values of each pixel in row major order under test 2
	std::ofstream ofsTest2("testing/test_2_cpp.txt", std::ofstream::out);
	ofsTest2 << h << std::endl;
	ofsTest2 << w << std::endl;

	for(uint i = 0; i < h; ++i)
	{
		for(uint j = 0; j < w; ++j)
		{
			//print r g b int values in new lines
			ofsTest2 << (int)pop->pixMap[i][j].r << std::endl;
			ofsTest2 << (int)pop->pixMap[i][j].g << std::endl;
			ofsTest2 << (int)pop->pixMap[i][j].b << std::endl;
		}
	}

	//close test_2_cpp.txt file
	ofsTest2.close();
	#endif
}

/*
1. Detects control points which are boundary(white) points where 3 or more regions meet
2. Initializes a graph with its vertices
*/
void Bitmap::detectControlPoints()
{
	uint h = pop->height;
	uint w = pop->width;
	uint vertexCounter = 0;
	std::vector<pixel> temp;
	pixel whitePixel;
	whitePixel.r = whitePixel.g = whitePixel.b = 255;

	#ifdef _TEST_4_
	//file to store coordinate values of control points under test 4
	std::ofstream ofsTest4("testing/test_4_cpp.txt", std::ofstream::out);
	ofsTest4 << h << std::endl;
	ofsTest4 << w << std::endl;
	#endif

	for(uint i = 1; i < h - 1; ++i)
	{
		for(uint j = 1; j < w - 1; ++j)
		{
			if(ifEqualPixel(pop->pixMap[i][j], whitePixel))
			{
				pixToNodeMap[i][j] = vertexCounter;
				graph->addVertex(vertexCounter, i, j, 0, 0, 0);
				temp.clear();
				if(checkUniqueRegionPixel(pop->pixMap[i][j - 1], temp))
					graph->addRegion(vertexCounter, codedImage->getMatrix()[i][j - 1]);

				if(checkUniqueRegionPixel(pop->pixMap[i][j + 1], temp))
					graph->addRegion(vertexCounter, codedImage->getMatrix()[i][j + 1]);

				if(checkUniqueRegionPixel(pop->pixMap[i - 1][j - 1], temp))
					graph->addRegion(vertexCounter, codedImage->getMatrix()[i - 1][j - 1]);

				if(checkUniqueRegionPixel(pop->pixMap[i - 1][j + 1], temp))
					graph->addRegion(vertexCounter, codedImage->getMatrix()[i - 1][j + 1]);

				if(checkUniqueRegionPixel(pop->pixMap[i + 1][j - 1], temp))
					graph->addRegion(vertexCounter, codedImage->getMatrix()[i + 1][j - 1]);

				if(checkUniqueRegionPixel(pop->pixMap[i + 1][j + 1], temp))
					graph->addRegion(vertexCounter, codedImage->getMatrix()[i + 1][j + 1]);

				if(checkUniqueRegionPixel(pop->pixMap[i - 1][j], temp))
					graph->addRegion(vertexCounter, codedImage->getMatrix()[i - 1][j]);

				if(checkUniqueRegionPixel(pop->pixMap[i + 1][j], temp))
					graph->addRegion(vertexCounter, codedImage->getMatrix()[i + 1][j]);


				if(temp.size() >= 3)
				{
					graph->setControlPoint(vertexCounter, 1);
					#ifdef _TEST_4_
					//print coordinates of control point
					ofsTest4 << i << std::endl;
					ofsTest4 << j << std::endl;
					#endif
				}

				vertexCounter++;
			}
			else
			{
				pixToNodeMap[i][j] = -1;
			}
		}
	}

	#ifdef _TEST_4_
	//close file
	ofsTest4.close();
	#endif

	temp.clear();
}

int Bitmap::checkUniqueRegionPixel(pixel a, std::vector<pixel> &v)
{
	int ret = 0;
	if(!(a.r == 255 && a.g == 255 && a.b == 255))
	{
		uint flag = 0;
		for(uint i = 0; i < v.size(); ++i)
		{
			if(ifEqualPixel(a, v[i]))
			{
				flag = 1;
				break;
			}
		}
		if(flag == 0)
		{
			v.push_back(a);
			ret = 1;
		}
	}
	
	return ret;
}

void Bitmap::formAdjacencyList()
{
	uint h = pop->height;
	uint w = pop->width;
	pixel whitePixel;
	whitePixel.r = whitePixel.g = whitePixel.b = 255;

	for(uint i = 1; i < h - 1; ++i)
	{
		for(uint j = 1; j < w - 1; ++j)
		{
			if(ifEqualPixel(pop->pixMap[i][j], whitePixel))
			{
				if(ifEqualPixel(pop->pixMap[i - 1][j], whitePixel))
					graph->addEdge(pixToNodeMap[i][j], pixToNodeMap[i - 1][j]);

				if(ifEqualPixel(pop->pixMap[i + 1][j], whitePixel))
					graph->addEdge(pixToNodeMap[i][j], pixToNodeMap[i + 1][j]);

				if(ifEqualPixel(pop->pixMap[i - 1][j - 1], whitePixel))
					graph->addEdge(pixToNodeMap[i][j], pixToNodeMap[i - 1][j - 1]);

				if(ifEqualPixel(pop->pixMap[i - 1][j + 1], whitePixel))
					graph->addEdge(pixToNodeMap[i][j], pixToNodeMap[i - 1][j + 1]);

				if(ifEqualPixel(pop->pixMap[i + 1][j - 1], whitePixel))
					graph->addEdge(pixToNodeMap[i][j], pixToNodeMap[i + 1][j - 1]);

				if(ifEqualPixel(pop->pixMap[i + 1][j + 1], whitePixel))
					graph->addEdge(pixToNodeMap[i][j], pixToNodeMap[i + 1][j + 1]);

				if(ifEqualPixel(pop->pixMap[i][j - 1], whitePixel))
					graph->addEdge(pixToNodeMap[i][j], pixToNodeMap[i][j - 1]);

				if(ifEqualPixel(pop->pixMap[i][j + 1], whitePixel))
					graph->addEdge(pixToNodeMap[i][j], pixToNodeMap[i][j + 1]);
			}
		}
	}
}
