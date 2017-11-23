#include <iostream>
#include "opencv_3.3.0.h"


using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
	Mat binary_frame = imread("findpupil.jpg", IMREAD_GRAYSCALE);
	namedWindow("test", WINDOW_NORMAL);
	imshow("test", binary_frame);

	//projection histogram을 위한 Mat 객체 생성
	Mat ver_proj(1, binary_frame.cols, CV_8U);
	ver_proj = Scalar::all(0);
	Mat hor_proj(binary_frame.rows, 1, CV_8U);
	hor_proj = Scalar::all(0);

	//projection result
	for (int i = 0; i < binary_frame.cols; i++)
	{
		ver_proj.at<uchar>(0, i) = countNonZero(binary_frame(Rect(i, 0, 1, binary_frame.rows)));
	}
	for (int i = 0; i < binary_frame.rows; i++)
	{
		hor_proj.at<uchar>(i, 0) = countNonZero(binary_frame(Rect(0, i, binary_frame.cols, 1)));
	}

	//indexing ver_proj
	for (int i = 1; i < ver_proj.cols; i++)
	{

	}

	waitKey(0);

	return 0;
}

