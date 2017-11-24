#include <iostream>
#include "opencv_3.3.0.h"

using namespace std;
using namespace cv;

#define VERTICAL 1
#define HORIZONTAL 2

typedef struct vIndex_data
{
	bool hasFPupil = false;
	int first_pupil[2];
	int second_pupil[2];
}vIndex_data;

typedef struct hIndex_data
{
	bool hasFPupil = false;
	int first_pupil[2];
	int second_pupil[2];
}hIndex_data;

void makeHistProj(Mat& src, Mat& dst, int direction);

int main(int argc, char* argv[])
{
	Mat binary_frame = imread("findpupil.jpg", IMREAD_GRAYSCALE);
	namedWindow("test", WINDOW_NORMAL);
	imshow("test", binary_frame);

	//finding indexing value using vertical histogram
	vIndex_data kkr_vIndex;
	Mat binary_vProj(1, binary_frame.cols, CV_8U);
	makeHistProj(binary_frame, binary_vProj, VERTICAL);

	for (int i = 1; i < binary_vProj.cols; i++)
	{
		if (kkr_vIndex.hasFPupil == false)
		{
			if (binary_vProj.at<uchar>(0, i) != 0 && binary_vProj.at<uchar>(0, i - 1) == 0)
			{
				kkr_vIndex.first_pupil[0] = i;
			}
			else if (binary_vProj.at<uchar>(0, i) == 0 && binary_vProj.at<uchar>(0, i - 1) != 0)
			{
				kkr_vIndex.first_pupil[1] = i - 1;
				kkr_vIndex.hasFPupil = true;
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
				kkr_vIndex.hasFPupil = false;
			}
		}
	}

	//°ª È®ÀÎ
	cout << "(" << kkr_vIndex.first_pupil[0] << "," << kkr_vIndex.first_pupil[1] << ")" << endl;
	cout << "(" << kkr_vIndex.second_pupil[0] << "," << kkr_vIndex.second_pupil[1] << ")" << endl;



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

	cout << "(" << kkr_hIndex.first_pupil[0] << "," << kkr_hIndex.first_pupil[1] << ")" << endl;

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

	Mat right_hProjR(right_pupil.rows, 1, CV_8U);
	for (int i = 0; i < right_pupil.rows; i++)
	{
		right_hProjR
	}

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

	cout << "(" << kkr_hIndex.second_pupil[0] << "," << kkr_hIndex.second_pupil[1] << ")" << endl;

	namedWindow("left", WINDOW_NORMAL);
	imshow("left", left_pupil);
	namedWindow("right", WINDOW_NORMAL);
	imshow("right", right_pupil);
	namedWindow("left_hist", WINDOW_NORMAL);
	imshow("left_hist", left_hProj);
	namedWindow("right_hist", WINDOW_NORMAL);
	imshow("right_hist", right_hProj);


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