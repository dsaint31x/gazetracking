#include <iostream>
#include "opencv_3.3.0.h"

#define FPS 60

using namespace std;
using namespace cv;

typedef struct mask
{
	bool isChecked;
	int x_offset;
	int y_offset;
	Point center;
	Point left_bottom;
	Point right_top;
}mask;

void CallBackFunc(int event, int x, int y, int flags, void* userdata);
void inverse(Mat& img);

int main(int argc, char* argv[])
{
	cout << "OpenCV Version : " << CV_VERSION << endl;
	cout << "�̰��� �������ּ���." << endl;

	//VideoCapture ��ü ����
	VideoCapture vid(0);
	if (!vid.isOpened()) {
		cerr << "open() error" << endl;
	}

	//������ ó���� ���� Mat ��ü ����
	Mat original_frame, copy_frame, selec_frame, gray_frame, binary_frame;

	//mask set
	mask kkr_mask;
	kkr_mask.isChecked = false;
	kkr_mask.x_offset = 80;
	kkr_mask.y_offset = 30;

	//window ���� and window callback �Լ� ���
	namedWindow("original_frame", WINDOW_KEEPRATIO);
	namedWindow("binary", WINDOW_KEEPRATIO);
	namedWindow("ver_proj", WINDOW_KEEPRATIO);
	namedWindow("hor_proj", WINDOW_KEEPRATIO);
	setMouseCallback("original_frame", CallBackFunc, &kkr_mask);

	//����ȭ�� ���� threshold trackbar ����
	int thresh = 30;
	namedWindow("select threshold", WINDOW_KEEPRATIO);
	cvCreateTrackbar("threshold", "select threshold", &thresh, 255);

	//control mask
	cout << "mask center move('8','4','5','6')" << endl;
	cout << "mask size control('+','-')" << endl;
	cout << "binary frame capture('s')" << endl;

	while (true)
	{
		//original frame���� ���� �޾ƿ��� �¿� ������ ���� ���
		vid >> original_frame;
		flip(original_frame, original_frame, 1);

		//�������� ���
		if (kkr_mask.isChecked == false)
		{
			imshow("original_frame", original_frame);
		}

		//face_roi�κ��� ���
		if (kkr_mask.isChecked == true)
		{
			copy_frame = original_frame.clone();
			rectangle(copy_frame, kkr_mask.left_bottom, kkr_mask.right_top, Scalar(0, 0, 255));
			selec_frame = original_frame(Rect(kkr_mask.left_bottom, kkr_mask.right_top)).clone();

			imshow("original_frame", copy_frame);

			//BRG -> grayscale -> binary
			cvtColor(selec_frame, gray_frame, CV_RGB2GRAY);
			threshold(gray_frame, binary_frame, thresh, 255, THRESH_BINARY);
			inverse(binary_frame);

			//morphological opening closing ���� ������ ���ſ� ������ ���� �޿�� 
			erode(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
			dilate(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
			dilate(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
			erode(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));

			//projection histogram�� ���� Mat ��ü ����
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

			imshow("binary", binary_frame);
			imshow("ver_proj", ver_proj);
			imshow("hor_proj", hor_proj);
		}

		int key = waitKey(1000 / FPS);
		if (key == 27)
		{
			break;
		}
		else if (key == 's')
		{
			imwrite("data\findpupil.jpg", binary_frame);
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