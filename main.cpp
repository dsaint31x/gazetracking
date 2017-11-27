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
#include "opencv2\core.hpp"

#define FPS 60
#define VERTICAL 1
#define HORIZONTAL 2

using namespace std;
using namespace cv;

typedef struct g_data
{
	bool hasPupil = false;
	Point firstPupil_leftTop;
	Point firstPupil_rightBottom;
	Point firstPupil_center;
	Point secondPupil_leftTop;
	Point secondPupil_rightBottom;
	Point secondPupil_center;
}g_data;

typedef struct mask
{
	bool isChecked;
	int x_offset;
	int y_offset;
	Point center;
	Point left_bottom;
	Point right_top;
}mask;

void inverse(Mat& img);
void makeHistProj(Mat& src, Mat& dst, int direction);
void getFirstCenter(Mat& vProj, Mat& hProj, g_data* p);
void getSecondCenter(Mat& vProj, Mat& hProj, g_data* p);
void CallBackFunc(int event, int x, int y, int flags, void* userdata);

int main(int argc, char* argv[])
{
	cout << "OpenCV Version : " << CV_VERSION << endl;
	cout << "미간을 선택해주세요." << endl;

	//VideoCapture 객체 생성
	VideoCapture vid(0);
	if (!vid.isOpened()) {
		cerr << "open() error" << endl;
	}

	//데이터 처리를 위한 Mat 객체 생성
	Mat original_frame, copy_frame, selec_frame, gray_frame, binary_frame;

	//mask size set
	mask kkr_mask;
	kkr_mask.isChecked = false;
	kkr_mask.x_offset = 80;
	kkr_mask.y_offset = 30;

	//window 생성 and window callback 함수 등록
	namedWindow("original_frame", WINDOW_NORMAL);
	namedWindow("binary", WINDOW_NORMAL);
	namedWindow("center", WINDOW_NORMAL);
	setMouseCallback("original_frame", CallBackFunc, &kkr_mask);

	//이진화를 위한 threshold trackbar 설정
	int thresh = 30;
	namedWindow("select threshold", WINDOW_KEEPRATIO);
	cvCreateTrackbar("threshold", "select threshold", &thresh, 255);
	
	//control mask
	cout << "mask center move('8','4','5','6')" << endl;
	cout << "mask size control('+','-')" << endl;
	cout << "binary frame capture('s')" << endl;

	while (true)
	{
		//original frame으로 영상 받아오고 좌우 반전된 영상 얻기
		vid >> original_frame;
		flip(original_frame, original_frame, 1);

		//원본영상 출력
		if (kkr_mask.isChecked == false)
		{
			copy_frame = original_frame.clone();
			imshow("original_frame", original_frame);
		}

		//face_roi부분을 출력
		if (kkr_mask.isChecked == true)
		{
			copy_frame = original_frame.clone();
			rectangle(copy_frame, kkr_mask.left_bottom, kkr_mask.right_top, Scalar(0, 0, 255));
			selec_frame = original_frame(Rect(kkr_mask.left_bottom, kkr_mask.right_top)).clone();

			imshow("original_frame", copy_frame);

			//BRG -> grayscale -> binarization -> inverse
			cvtColor(selec_frame, gray_frame, CV_RGB2GRAY);
			threshold(gray_frame, binary_frame, thresh, 255, THRESH_BINARY);
			inverse(binary_frame);

			//morphological opening -> closing 작은 점들을 제거와 영역의 구멍 메우기 
			erode(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
			dilate(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
			dilate(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
			erode(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
			
			//g_data 구조체 변수와 구조체 포인터 변수 생성
			//구조체에는 동공의 좌표의 index를 저장할 Point 변수 6개가 존재한다.
			g_data kkr_pupilIndex;
			g_data* kkr_pupilPointer = &kkr_pupilIndex;

			//vertical projection한 Mat 객체 생성
			//(src, dst, direction)
			Mat binary_vProj(1, binary_frame.cols, CV_8U);
			makeHistProj(binary_frame, binary_vProj, VERTICAL);
			
			/*
			동공 index 저장
			hasPupil 변수는 frame에서 한 개의 동공만 존재할 때, 처리하기 위해 설정한다.
			binary_vProj은 현재 두개의 봉오리를 가지고 있다.
			binary_vProj.at<uchar>(0, i)가 값이 변할 때,
			i를 firstPupil_leftTop.x, firstPupil_rightBotto=mp.x, secondPupil_leftTop.x, secondPupil_rightBottom.x를 저장한다.
			*/
			for (int i = 1; i < binary_vProj.cols; i++)
			{
				if (kkr_pupilIndex.hasPupil == false)
				{
					if (binary_vProj.at<uchar>(0, i) != 0 && binary_vProj.at<uchar>(0, i - 1) == 0)
					{
						kkr_pupilIndex.firstPupil_leftTop.x = i;
					}
					else if (binary_vProj.at<uchar>(0, i) == 0 && binary_vProj.at<uchar>(0, i - 1) != 0)
					{
						kkr_pupilIndex.firstPupil_rightBottom.x = i - 1;
						kkr_pupilIndex.hasPupil = true;
					}
				}
				else if (kkr_pupilIndex.hasPupil == true)
				{
					if (binary_vProj.at<uchar>(0, i) != 0 && binary_vProj.at<uchar>(0, i - 1) == 0)
					{
						kkr_pupilIndex.secondPupil_leftTop.x = i;
					}
					else if (binary_vProj.at<uchar>(0, i) == 0 && binary_vProj.at<uchar>(0, i - 1) != 0)
					{
						kkr_pupilIndex.secondPupil_rightBottom.x = i - 1;
						kkr_pupilIndex.hasPupil = false;
					}
				}
			}

			/*
			동공 index 저장
			hasPupil 변수는 frame에서 한 개의 동공만 존재할 때, 처리하기 위해 설정했다.
			두 개의 동공을 갖는 binary_vProj은 x축에서 두개의 봉오리를 가지고 있다.
			binary_vProj.at<uchar>(0, i)가 값이 변할 때,
			i를 firstPupil_leftTop.x, firstPupil_rightBottom.x, secondPupil_leftTop.x, secondPupil_rightBottom.x를 저장한다.
			*/
			Mat left_pupil = binary_frame.clone();
			for (int i = 0; i < left_pupil.rows; i++)
			{
				uchar* p = left_pupil.ptr<uchar>(i);
				for (int j = 0; j < left_pupil.cols; j++)
				{
					if (j >= kkr_pupilIndex.firstPupil_leftTop.x && j <= kkr_pupilIndex.firstPupil_rightBottom.x)
					{
						continue;
					}
					p[j] = 0;
				}
			}

			//Mat 객체 left_pupil을 각각 vertical, horizontal projection histogram 한다. 
			Mat left_vProj(1, left_pupil.cols, CV_8U);
			makeHistProj(left_pupil, left_vProj, VERTICAL);
			Mat left_hProj(1, left_pupil.rows, CV_8U);
			makeHistProj(left_pupil, left_hProj, HORIZONTAL);

			/*
			left_pupil Mat 객체를 horizontal projection 했기 때문에 y 축에서 봉오리 하나를 갖는다.
			따라서 binary_hProj.at<uchar>(0, i)가 값이 변할 때,
			i를 firstPupil_leftTop.y, firstPupil_rightBottom.y에 저장한다.
			*/
			for (int i = 1; i < left_hProj.cols; i++)
			{
				if (left_hProj.at<uchar>(0, i) != 0 && left_hProj.at<uchar>(0, i - 1) == 0)
				{
					kkr_pupilIndex.firstPupil_leftTop.y = i;
				}
				else if (left_hProj.at<uchar>(0, i) == 0 && left_hProj.at<uchar>(0, i - 1) != 0)
				{
					kkr_pupilIndex.firstPupil_rightBottom.y = i - 1;
				}
			}

			//make only right pupil Mat
			
			Mat right_pupil = binary_frame.clone();
			for (int i = 0; i < right_pupil.rows; i++)
			{
				uchar* p = right_pupil.ptr<uchar>(i);
				for (int j = 0; j < right_pupil.cols; j++)
				{
					if (j >= kkr_pupilIndex.secondPupil_leftTop.x && j <= kkr_pupilIndex.secondPupil_rightBottom.x)
					{
						continue;
					}
					p[j] = 0;
				}
			}

			//finding indexing value using horizontal histogram
			Mat right_vProj(1, right_pupil.cols, CV_8U);
			makeHistProj(right_pupil, right_vProj, VERTICAL);
			Mat right_hProj(1, right_pupil.rows, CV_8U);
			makeHistProj(right_pupil, right_hProj, HORIZONTAL);

			//indexing hor_proj
			for (int i = 1; i < right_hProj.cols; i++)
			{
				if (right_hProj.at<uchar>(0, i) != 0 && right_hProj.at<uchar>(0, i - 1) == 0)
				{
					kkr_pupilIndex.secondPupil_leftTop.y = i;
				}
				else if (right_hProj.at<uchar>(0, i) == 0 && right_hProj.at<uchar>(0, i - 1) != 0)
				{
					kkr_pupilIndex.secondPupil_rightBottom.y = i - 1;
				}
			}

			getFirstCenter(left_vProj, left_hProj, kkr_pupilPointer);
			getSecondCenter(right_vProj, right_hProj, kkr_pupilPointer);


			//중심점에서 사각형 그리기
			rectangle(binary_frame, Rect(Point(kkr_pupilIndex.firstPupil_center.x - 8, kkr_pupilIndex.firstPupil_center.y - 7), Point(kkr_pupilIndex.firstPupil_center.x + 8, kkr_pupilIndex.firstPupil_center.y + 8)), Scalar(255, 255, 255));
			rectangle(binary_frame, Rect(Point(kkr_pupilIndex.secondPupil_center.x - 8, kkr_pupilIndex.secondPupil_center.y - 7), Point(kkr_pupilIndex.secondPupil_center.x + 8, kkr_pupilIndex.secondPupil_center.y + 8)), Scalar(255, 255, 255));

			//동공이 나타나는 index 값을 통해 사각형 그리기
			//rectangle(binary_frame, Rect(Point(kkr_vIndex.first_pupil[0], kkr_hIndex.first_pupil[0]), Point(kkr_vIndex.first_pupil[1], kkr_hIndex.first_pupil[1])), Scalar(255, 255, 255));
			//rectangle(binary_frame, Rect(Point(kkr_vIndex.second_pupil[0], kkr_hIndex.second_pupil[0]), Point(kkr_vIndex.second_pupil[1], kkr_hIndex.second_pupil[1])), Scalar(255, 255, 255));
			
			imshow("center", binary_frame);
		}

		int key = waitKey(1000 / FPS);
		if (key == 27)
		{
			break;
		}
		else if (key == 's')
		{
			imwrite("findpupil.png", binary_frame);
		}
	}

	return 0;
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	mask* kkr_mask = (mask*)userdata;
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
		kkr_mask->isChecked = true;
		kkr_mask->center = { x,y };
		kkr_mask->left_bottom = { kkr_mask->center.x - kkr_mask->x_offset,kkr_mask->center.y - kkr_mask->y_offset };
		kkr_mask->right_top = { kkr_mask->center.x + kkr_mask->x_offset,kkr_mask->center.y + kkr_mask->y_offset };
	}

	if (event == CV_EVENT_RBUTTONDOWN)
	{
		kkr_mask->isChecked = false;
	}
}

void inverse(Mat& img)
{
	if (img.channels() == 1)
	{
		for (int i = 0; i < img.rows; i++)
		{
			uchar* p = img.ptr<uchar>(i);
			for (int j = 0; j < img.cols; j++)
			{
				p[j] = 255 - p[j];
			}
		}
	}
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

//무게중심 공식 center = sum(좌표*좌표에서 histogram value)/sum(histogram value)
void getFirstCenter(Mat& vProj, Mat& hProj, g_data* p)
{
	int numerator = 0;
	int denominator = 0;

	for (int i = p->firstPupil_leftTop.x; i <= p->firstPupil_rightBottom.x; i++)
	{
		numerator = numerator + vProj.at<uchar>(0, i)*i;
		denominator = denominator + vProj.at<uchar>(0, i);
	}

	if (numerator != 0 && denominator != 0)
	{
		p->firstPupil_center.x = numerator / denominator;
	}

	numerator = 0;
	denominator = 0;

	for (int i = p->firstPupil_leftTop.y; i <= p->firstPupil_rightBottom.y; i++)
	{
		numerator = numerator + hProj.at<uchar>(0, i)*i;
		denominator = denominator + hProj.at<uchar>(0, i);
	}

	if (numerator != 0 && denominator != 0)
	{
		p->firstPupil_center.y = numerator / denominator;
	}

	numerator = 0;
	denominator = 0;
}

//무게중심 공식 center = sum(좌표*좌표에서 histogram value)/sum(histogram value)
void getSecondCenter(Mat& vProj, Mat& hProj, g_data* p)
{
	int numerator = 0;
	int denominator = 0;

	for (int i = p->secondPupil_leftTop.x; i <= p->secondPupil_rightBottom.x; i++)
	{
		numerator = numerator + vProj.at<uchar>(0, i)*i;
		denominator = denominator + vProj.at<uchar>(0, i);
	}
	
	if (numerator != 0 && denominator != 0)
	{
		p->secondPupil_center.x = numerator / denominator;
		numerator = 0;
		denominator = 0;
	}
	

	for (int i = p->secondPupil_leftTop.y; i <= p->secondPupil_rightBottom.y; i++)
	{
		numerator = numerator + hProj.at<uchar>(0, i)*i;
		denominator = denominator + hProj.at<uchar>(0, i);
	}

	if (numerator != 0 && denominator != 0)
	{
		p->secondPupil_center.y = numerator / denominator;
		numerator = 0;
		denominator = 0;
	}
}