#include <iostream>
#include "opencv_3.3.0.h"

#define VERTICAL 1
#define HORIZONTAL 2

using namespace std;
using namespace cv;

typedef struct vIndex_data
{
	bool hasPupil = false;
	int first_pupil[2];
	int first_center;
	int second_pupil[2];
	int second_center;
}vIndex_data;

typedef struct hIndex_data
{
	bool hasPupil = false;
	int first_pupil[2];
	int first_center;
	int second_pupil[2];
	int second_center;
}hIndex_data;

void makeHistProj(Mat& src, Mat& dst, int direction);

int main(int argc, char* argv[])
{
	Mat binary_frame = imread("findpupil.jpg", IMREAD_GRAYSCALE);
	namedWindow("test", WINDOW_NORMAL);
	//imshow("test", binary_frame);

	//finding indexing value using vertical histogram
	vIndex_data kkr_vIndex;
	Mat binary_vProj(1, binary_frame.cols, CV_8U);
	makeHistProj(binary_frame, binary_vProj, VERTICAL);


	for (int i = 1; i < binary_vProj.cols; i++)
	{
		if (kkr_vIndex.hasPupil == false)
		{
			if (binary_vProj.at<uchar>(0, i) != 0 && binary_vProj.at<uchar>(0, i - 1) == 0)
			{
				kkr_vIndex.first_pupil[0] = i;
			}
			else if (binary_vProj.at<uchar>(0, i) == 0 && binary_vProj.at<uchar>(0, i - 1) != 0)
			{
				kkr_vIndex.first_pupil[1] = i - 1;
				kkr_vIndex.hasPupil = true;
			}
		}
		else
		{
			if (binary_vProj.at<uchar>(0, i) != 0 && binary_vProj.at<uchar>(0, i - 1) == 0)
			{
				kkr_vIndex.second_pupil[0] = i;
			}
			else if (binary_vProj.at<uchar>(0, i) == 0 && binary_vProj.at<uchar>(0, i - 1) != 0)
			{
				kkr_vIndex.second_pupil[1] = i-1;
				kkr_vIndex.hasPupil = false;
			}
		}
	}

	//make only left pupil Mat
	Mat left_pupil = binary_frame.clone();
	for (int i = 0; i < left_pupil.rows; i++)
	{
		uchar* p = left_pupil.ptr<uchar>(i);
		for (int j = 0; j < left_pupil.cols; j++)
		{
			if (j >= kkr_vIndex.first_pupil[0] && j <= kkr_vIndex.first_pupil[1])
			{
				continue;
			}
			p[j] = 0;
		}
	}

	//finding indexing value using horizontal histogram
	hIndex_data kkr_hIndex;
	Mat left_hProj(1, left_pupil.rows, CV_8U);
	makeHistProj(left_pupil, left_hProj, HORIZONTAL);

	//indexing hor_proj
	for (int i = 1; i < left_hProj.cols; i++)
	{
		if (left_hProj.at<uchar>(0, i) != 0 && left_hProj.at<uchar>(0, i - 1) == 0)
		{
			kkr_hIndex.first_pupil[0]= i;
		}
		else if (left_hProj.at<uchar>(0, i) == 0 && left_hProj.at<uchar>(0, i - 1) != 0)
		{
			kkr_hIndex.first_pupil[1] = i - 1;
		}
	}

	//make only right pupil Mat
	Mat right_pupil = binary_frame.clone();
	for (int i = 0; i < right_pupil.rows; i++)
	{
		uchar* p = right_pupil.ptr<uchar>(i);
		for (int j = 0; j < right_pupil.cols; j++)
		{
			if (j >= kkr_vIndex.second_pupil[0] && j <= kkr_vIndex.second_pupil[1])
			{
				continue;
			}
			p[j] = 0;
		}
	}

	//finding indexing value using horizontal histogram
	Mat right_hProj(1, right_pupil.rows, CV_8U);
	makeHistProj(right_pupil, right_hProj, HORIZONTAL);

	//indexing hor_proj
	for (int i = 1; i < right_hProj.cols; i++)
	{
		if (right_hProj.at<uchar>(0, i) != 0 && right_hProj.at<uchar>(0, i - 1) == 0)
		{
			kkr_hIndex.second_pupil[0] = i;
		}
		else if (right_hProj.at<uchar>(0, i) == 0 && right_hProj.at<uchar>(0, i - 1) != 0)
		{
			kkr_hIndex.second_pupil[1] = i - 1;
		}
	}

	int numerator = 0;
	int denominator = 0;

	for (int i = kkr_vIndex.first_pupil[0]; i <= kkr_vIndex.first_pupil[1]; i++)
	{
		numerator = numerator + binary_vProj.at<uchar>(0, i)*i;
		denominator = denominator + binary_vProj.at<uchar>(0, i);
	}

	kkr_vIndex.first_center = (int)numerator / denominator;
	numerator = 0;
	denominator = 0;

	for (int i = kkr_vIndex.second_pupil[0]; i <= kkr_vIndex.second_pupil[1]; i++)
	{
		numerator = numerator + binary_vProj.at<uchar>(0, i)*i;
		denominator = denominator + binary_vProj.at<uchar>(0, i);
	}

	kkr_vIndex.second_center = numerator / denominator;
	numerator = 0;
	denominator = 0;

	for (int i = kkr_hIndex.first_pupil[0]; i <= kkr_hIndex.first_pupil[1]; i++)
	{
		numerator = numerator + left_hProj.at<uchar>(0, i)*i;
		denominator = denominator + left_hProj.at<uchar>(0, i);
	}

	kkr_hIndex.first_center = numerator / denominator;
	numerator = 0;
	denominator = 0;

	for (int i = kkr_hIndex.second_pupil[0]; i <= kkr_hIndex.second_pupil[1]; i++)
	{
		numerator = numerator + right_hProj.at<uchar>(0, i)*i;
		denominator = denominator + right_hProj.at<uchar>(0, i);
	}

	kkr_hIndex.second_center = numerator / denominator;
	numerator = 0;
	denominator = 0;

	cout << "first pupil x (" << kkr_vIndex.first_pupil[0] << "," << kkr_vIndex.first_pupil[1] << ")" << endl;
	cout << "first pupil y (" << kkr_hIndex.first_pupil[0] << "," << kkr_hIndex.first_pupil[1] << ")" << endl;
	cout << "second pupil x (" << kkr_vIndex.second_pupil[0] << "," << kkr_vIndex.second_pupil[1] << ")" << endl;
	cout << "second pupil y (" << kkr_hIndex.second_pupil[0] << "," << kkr_hIndex.second_pupil[1] << ")" << endl;


	cout << "first pupil (" << kkr_vIndex.first_center << "," << kkr_hIndex.first_center << ")" << endl;
	cout << "second pupil (" << kkr_vIndex.second_center << "," << kkr_hIndex.second_center << ")" << endl;


	rectangle(binary_frame, Rect(Point(kkr_vIndex.first_center - 8, kkr_hIndex.first_center - 8), Point(kkr_vIndex.first_center + 8, kkr_hIndex.first_center + 8)), Scalar(255, 255, 255));
	rectangle(binary_frame, Rect(Point(kkr_vIndex.second_center - 8, kkr_hIndex.second_center - 8), Point(kkr_vIndex.second_center + 8, kkr_hIndex.second_center + 8)), Scalar(255, 255, 255));
	//rectangle(binary_frame, Rect(Point(kkr_vIndex.first_pupil[0], kkr_hIndex.first_pupil[0]), Point(kkr_vIndex.first_pupil[1], kkr_hIndex.first_pupil[1])), Scalar(255, 255, 255));
	//rectangle(binary_frame, Rect(Point(kkr_vIndex.second_pupil[0], kkr_hIndex.second_pupil[0]), Point(kkr_vIndex.second_pupil[1], kkr_hIndex.second_pupil[1])), Scalar(255, 255, 255));

	imshow("test", binary_frame);


	waitKey(0);

	return 0;
}

void makeHistProj(Mat& src, Mat& dst, int direction)
{
	if (direction == 1)
	{
		for (int i = 0; i < src.cols; i++)
		{
			dst.at<uchar>(0, i) = countNonZero(src(Rect(i, 0, 1, src.rows)));
		}
	}
	else if (direction == 2)
	{
		for (int i = 0; i < src.rows; i++)
		{
			dst.at<uchar>(0, i) = countNonZero(src(Rect(0, i, src.cols, 1)));
		}
	}
}