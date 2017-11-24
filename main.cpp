#include <iostream>
#include "opencv_3.3.0.h"

using namespace std;
using namespace cv;

#define VERTICAL 1
#define HORIZONTAL 2

typedef struct ver_index_data
{
	bool hasFPupil = false;
	int first_pupil[2];
	int second_pupil[2];
}pupil_vertical_index_data;

typedef struct hor_index_data
{
	bool hasFPupil = false;
	int first_pupil[2];
	int second_pupil[2];
}hor_index_data;

void makeHistProj(Mat& src, Mat& dst, int direction);

int main(int argc, char* argv[])
{
	Mat binary_frame = imread("findpupil.jpg", IMREAD_GRAYSCALE);
	namedWindow("test", WINDOW_NORMAL);
	imshow("test", binary_frame);

	//projection histogram을 위한 Mat 객체 생성
	Mat binary_ver_proj(1, binary_frame.cols, CV_8U);


	//projection result


	//pupil_vertical_index_data kkr_ver_index;
	//pupil_horizontal_index_data kkr_hor_index;

	////indexing ver_proj
	//for (int i = 1; i < ver_proj.cols; i++)
	//{
	//	if (kkr_ver_index.hasFPupil == false)
	//	{
	//		if (ver_proj.at<uchar>(0, i) != 0 && ver_proj.at<uchar>(0, i - 1) == 0)
	//		{
	//			kkr_ver_index.first_Lindex = i;
	//		}
	//		else if (ver_proj.at<uchar>(0, i) == 0 && ver_proj.at<uchar>(0, i - 1) != 0)
	//		{
	//			kkr_ver_index.first_Rindex = i - 1;
	//			kkr_ver_index.hasFPupil = true;
	//		}
	//	}
	//	else
	//	{
	//		if (ver_proj.at<uchar>(0, i) != 0 && ver_proj.at<uchar>(0, i - 1) == 0)
	//		{
	//			kkr_ver_index.second_Lindex = i;
	//		}
	//		else if (ver_proj.at<uchar>(0, i) == 0 && ver_proj.at<uchar>(0, i - 1) != 0)
	//		{
	//			kkr_ver_index.second_Rindex = i-1;
	//			kkr_ver_index.hasFPupil = true;
	//		}
	//	}
	//}

	//cout << "(" << kkr_ver_index.first_Lindex << "," << kkr_ver_index.first_Rindex << ")" << endl;
	//cout << "(" << kkr_ver_index.second_Lindex << "," << kkr_ver_index.second_Rindex << ")" << endl;

	////left pupil Mat
	//Mat left_puppil_frame = binary_frame.clone();
	//for (int i = 0; i < left_puppil_frame.rows; i++)
	//{
	//	uchar* p = left_puppil_frame.ptr<uchar>(i);
	//	for (int j = 0; j < left_puppil_frame.cols; j++)
	//	{
	//		if (j >= kkr_ver_index.first_Lindex&&j <= kkr_ver_index.first_Rindex)
	//		{
	//			continue;
	//		}
	//		p[j] = 0;
	//	}
	//}

	////projection histogram을 위한 Mat 객체 생성
	//Mat hor_left_proj(left_puppil_frame.rows, 1, CV_8U);
	//hor_left_proj = Scalar::all(0);

	////left eye projection result
	//for (int i = 0; i < left_puppil_frame.rows; i++)
	//{
	//	hor_left_proj.at<uchar>(i, 0) = countNonZero(left_puppil_frame(Rect(0, i, left_puppil_frame.cols, 1)));
	//}


	////indexing hor_proj
	//for (int i = 1; i < hor_left_proj.rows; i++)
	//{
	//	if (hor_left_proj.at<uchar>(i, 0) != 0 && hor_left_proj.at<uchar>(i - 1, 0) == 0)
	//	{
	//		kkr_hor_index.first_Tindex= i;
	//	}
	//	else if (hor_left_proj.at<uchar>(i, 0) == 0 && hor_left_proj.at<uchar>(i - 1, 0) != 0)
	//	{
	//		kkr_hor_index.first_Bindex = i - 1;
	//	}
	//}

	//cout << "(" << kkr_hor_index.first_Tindex << "," << kkr_hor_index.first_Bindex << ")" << endl;

	////right pupil Mat
	//Mat right_puppil_frame = binary_frame.clone();
	//for (int i = 0; i < right_puppil_frame.rows; i++)
	//{
	//	uchar* p = right_puppil_frame.ptr<uchar>(i);
	//	for (int j = 0; j < right_puppil_frame.cols; j++)
	//	{
	//		if (j >= kkr_ver_index.second_Lindex&&j <= kkr_ver_index.second_Rindex)
	//		{
	//			continue;
	//		}
	//		p[j] = 0;
	//	}
	//}

	////projection histogram을 위한 Mat 객체 생성
	//Mat hor_right_proj(right_puppil_frame.rows, 1, CV_8U);
	//hor_right_proj = Scalar::all(0);

	////right eye projection result
	//for (int i = 0; i < right_puppil_frame.rows; i++)
	//{
	//	hor_right_proj.at<uchar>(i, 0) = countNonZero(right_puppil_frame(Rect(0, i, right_puppil_frame.cols, 1)));
	//}

	////indexing hor_proj
	//for (int i = 1; i < hor_right_proj.rows; i++)
	//{
	//	if (hor_right_proj.at<uchar>(i, 0) != 0 && hor_right_proj.at<uchar>(i - 1, 0) == 0)
	//	{
	//		kkr_hor_index.second_Tindex = i;
	//	}
	//	else if (hor_right_proj.at<uchar>(i, 0) == 0 && hor_right_proj.at<uchar>(i - 1, 0) != 0)
	//	{
	//		kkr_hor_index.second_Bindex = i - 1;
	//	}
	//}

	//cout << "(" << kkr_hor_index.second_Tindex << "," << kkr_hor_index.second_Bindex << ")" << endl;

	//namedWindow("left", WINDOW_NORMAL);
	//imshow("left", left_puppil_frame);
	//namedWindow("right", WINDOW_NORMAL);
	//imshow("right", right_puppil_frame);


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