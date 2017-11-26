/*

	전처리
	1. mousecallback함수를 통해 ROI 영역을 지정해준다.
	2. ROI 영역을 BRG -> grayscale -> binarization 한다.
	3. 계산의 편리를 위해 pixel 데이터 값을 inverse 해준다.
	4. 조명, 거리에 따라 동공 외 눈썹, 머리카락이 같이 검출될 수 있지만 동공 영역만 검출된 이미지 하나에서 동공을 검출 하려고 한다.

	본과정
	1. vertical projection을 통해서 두 눈이 시작되는 index와 끝나는 index를 저장한다.
	2. index value를 통해서 첫번째 눈만 있는 Mat 객체와 두번째 눈만 있는 Mat객체를 생성한다.
	3. 첫번째 Mat 객체와 두번째 Mat객체를 각각 horizontal projection을 이용해서 첫번째 눈과 두번째 눈 index값을 저장한다.
	4. 무게중심 공식 center = sum(좌표*좌표에서 histogram value)/sum(histogram value)
	5. 중심 좌표를 중심으로 사각형을 그린다.

	to do list
	0. 데이터 저장 png 또는 bmp파일로 하기
	1. 변수명, 구조체 알아보기 쉽게 수정(정리가 안됨)
	2. 중심점 좌표를 int형으로 했기 때문에 소수점이 버려진다. 이 경우 픽셀의 center 값을 2x2를 중심으로 설정하도록 한다.
	3. 중심점 구하는 함수 생성
	4. 눈 이외의 검출된 노이즈를 제거하기 위해 mean filter 적용하기
	5. 경우의 수 처리하기
		a. 눈썹과 머리카락이 함께 검출된 경우
		b. 한쪽 눈만 검출 된 경우

*/
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

	//vertical projection한 Mat 객체 생성
	vIndex_data kkr_vIndex;
	Mat binary_vProj(1, binary_frame.cols, CV_8U);
	makeHistProj(binary_frame, binary_vProj, VERTICAL);

	//동공 index 저장
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



	//중심점을 찾기 위한 계산
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

	//중심점에서 사각형 그리기
	rectangle(binary_frame, Rect(Point(kkr_vIndex.first_center - 8, kkr_hIndex.first_center - 8), Point(kkr_vIndex.first_center + 8, kkr_hIndex.first_center + 8)), Scalar(255, 255, 255));
	rectangle(binary_frame, Rect(Point(kkr_vIndex.second_center - 8, kkr_hIndex.second_center - 8), Point(kkr_vIndex.second_center + 8, kkr_hIndex.second_center + 8)), Scalar(255, 255, 255));
	
	//동공이 나타나는 index 값을 통해 사각형 그리기
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