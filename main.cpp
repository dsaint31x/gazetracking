#include <iostream>
#include "opencv_3.3.0.h"


using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
	Mat binary_frame = imread("findpupil.jpg", IMREAD_GRAYSCALE);
	namedWindow("test", WINDOW_NORMAL);
	imshow("test", binary_frame);

	waitKey(0);

	return 0;
}

