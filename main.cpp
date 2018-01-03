/*
	��ó��
	1. mousecallback�Լ��� ���� ROI ������ �������ش�.
	2. ROI ������ BRG -> grayscale -> binarization �Ѵ�.
	3. ����� ���� ���� pixel ������ ���� inverse ���ش�.
	4. ����, �Ÿ��� ���� ���� �� ����, �Ӹ�ī���� ���� ����� �� ������ ���� ������ ����� �̹��� �ϳ����� ������ ���� �Ϸ��� �Ѵ�.

	������
	1. vertical projection�� ���ؼ� �� ���� ���۵Ǵ� index�� ������ index�� �����Ѵ�.
	2. index value�� ���ؼ� ù��° ���� �ִ� Mat ��ü�� �ι�° ���� �ִ� Mat��ü�� �����Ѵ�.
	3. ù��° Mat ��ü�� �ι�° Mat��ü�� ���� horizontal projection�� �̿��ؼ� ù��° ���� �ι�° �� index���� �����Ѵ�.
	4. �����߽� ���� center = sum(��ǥ*��ǥ���� histogram value)/sum(histogram value)
	5. �߽� ��ǥ�� �߽����� �簢���� �׸���.

	to do list
	0. ������ ���� png �Ǵ� bmp���Ϸ� �ϱ�
	1. ������, ����ü �˾ƺ��� ���� ����(������ �ȵ�)
	2. �߽��� ��ǥ�� int������ �߱� ������ �Ҽ����� ��������. �� ��� �ȼ��� center ���� 2x2�� �߽����� �����ϵ��� �Ѵ�.
	3. �߽��� ���ϴ� �Լ� ����
	4. �� �̿��� ����� ����� �����ϱ� ���� mean filter �����ϱ�
	5. ����� �� ó���ϱ�
		a. ����� �Ӹ�ī���� �Բ� ����� ���
		b. ���� ���� ���� �� ���
*/

#include <iostream>
#include "opencv_3.3.0.h"

#define FPS 60
#define VERTICAL 1
#define HORIZONTAL 2

using namespace std;
using namespace cv;

typedef struct mask
{
	bool isChecked = false;
	int x_offset;
	int y_offset;
	Point center;
	Point left_top;
	Point right_bottom;
}mask;

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

typedef struct stimulation
{
	bool stimualtion = false;
	int x=0;
	int y=0;
}stimulation;

void inverse(Mat& img);
void makeHistProj(Mat& src, Mat& dst, int direction);
void getFirstCenter(Mat& vProj, Mat& hProj, g_data* p);
void getSecondCenter(Mat& vProj, Mat& hProj, g_data* p);
void CallBackFunc(int event, int x, int y, int flags, void* userdata);

int main(int argc, char* argv[])
{
	cout << "�̰��� �������ּ���." << endl;

	//VideoCapture ��ü ����
	VideoCapture vid(0);
	if (!vid.isOpened()) {
		cerr << "open() error" << endl;
	}

	//������ ó���� ���� Mat ��ü ����
	Mat original_frame, copy_frame, selec_frame, gray_frame, binary_frame;

	//mask size set
	mask kkr_mask;
	kkr_mask.x_offset = 80;
	kkr_mask.y_offset = 30;

	stimulation kkr_stimulation;

	//window ���� and window callback �Լ� ���
	namedWindow("original_frame", WINDOW_NORMAL);
	namedWindow("center", WINDOW_NORMAL);
	setMouseCallback("original_frame", CallBackFunc, &kkr_mask);

	//����ȭ�� ���� threshold trackbar ����
	int thresh = 30;
	namedWindow("select threshold", WINDOW_KEEPRATIO);
	cvCreateTrackbar("threshold", "select threshold", &thresh, 255);

	while (true)
	{
		//original frame���� ���� �޾ƿ��� �¿� ������ ���� ���
		vid >> original_frame;
		flip(original_frame, original_frame, 1);
		//���� frame�� copy_frame�� ����
		copy_frame = original_frame.clone();

		//�������� ���
		if (kkr_mask.isChecked == false)
		{
			imshow("original_frame", copy_frame);
		}

		//face_roi�κ��� ���
		if (kkr_mask.isChecked == true)
		{
			//input array�� �簢���� �׷��ִ� �Լ�
			rectangle(copy_frame, kkr_mask.left_top, kkr_mask.right_bottom, Scalar(0, 0, 255));
			//roi ������ ����
			selec_frame = original_frame(Rect(kkr_mask.left_top, kkr_mask.right_bottom)).clone();

			//imshow("original_frame", copy_frame);

			//BRG -> grayscale -> binarization -> inverse
			cvtColor(selec_frame, gray_frame, CV_RGB2GRAY);
			threshold(gray_frame, binary_frame, thresh, 255, THRESH_BINARY);
			inverse(binary_frame);

			//morphological opening -> closing ���� ������ ���ſ� ������ ���� �޿�� 
			erode(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
			dilate(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
			dilate(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
			erode(binary_frame, binary_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
			
			//g_data ����ü ������ ����ü ������ ���� ����
			//����ü���� ������ ��ǥ�� index�� ������ Point ���� 6���� �����Ѵ�.
			g_data kkr_pupilIndex;
			g_data* kkr_pupilPointer = &kkr_pupilIndex;

			//vertical projection�� Mat ��ü ����
			//(src, dst, direction)
			Mat binary_vProj(1, binary_frame.cols, CV_8U);
			makeHistProj(binary_frame, binary_vProj, VERTICAL);
			
			/*
			���� x index ����
			hasPupil ������ frame���� �� ���� ������ ������ ��, ó���ϱ� ���� �����Ѵ�.
			binary_vProj�� ���� �ΰ��� �������� ������ �ִ�.
			binary_vProj.at<uchar>(0, i)�� ���� ���� ��,
			i�� firstPupil_leftTop.x, firstPupil_rightBotto=mp.x, secondPupil_leftTop.x, secondPupil_rightBottom.x�� �����Ѵ�.
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
			���� ������ ���� Mat ��ü ����
			firstPupil_leftTop.x <= j <= firstPupil_rightBottom.x ���̿� �ִ� ���� continue��
			���ؼ� ���� �츮�� �̿��� �κп����� �����͸� ���� 0���� �ٲپ� �ش�.
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

			//Mat ��ü left_pupil�� ���� vertical, horizontal projection histogram �Ѵ�. 
			Mat left_vProj(1, left_pupil.cols, CV_8U);
			makeHistProj(left_pupil, left_vProj, VERTICAL);
			Mat left_hProj(1, left_pupil.rows, CV_8U);
			makeHistProj(left_pupil, left_hProj, HORIZONTAL);

			/*
			left_pupil Mat ��ü�� horizontal projection �߱� ������ y �࿡�� ������ �ϳ��� ���´�.
			���� binary_hProj.at<uchar>(0, i)�� ���� ���� ��,
			i�� firstPupil_leftTop.y, firstPupil_rightBottom.y�� �����Ѵ�.
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

			/*
			������ ������ ���� Mat ��ü ����
			secondPupil_leftTop.x <= j <= secondPupil_rightBottom.x ���̿� �ִ� ���� continue��
			���ؼ� ���� �츮�� �̿��� �κп����� �����͸� ���� 0���� �ٲپ� �ش�.
			*/
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

			//Mat ��ü right_pupil�� ���� vertical, horizontal projection histogram �Ѵ�.
			Mat right_vProj(1, right_pupil.cols, CV_8U);
			makeHistProj(right_pupil, right_vProj, VERTICAL);
			Mat right_hProj(1, right_pupil.rows, CV_8U);
			makeHistProj(right_pupil, right_hProj, HORIZONTAL);

			/*
			right_pupil Mat ��ü�� horizontal projection �߱� ������ y �࿡�� ������ �ϳ��� ���´�.
			���� binary_hProj.at<uchar>(0, i)�� ���� ���� ��,
			i�� secondPupil_leftTop.y, secondPupil_rightBottom.y�� �����Ѵ�.
			*/
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

			//�߽��� ã���Լ�
			getFirstCenter(left_vProj, left_hProj, kkr_pupilPointer);
			getSecondCenter(right_vProj, right_hProj, kkr_pupilPointer);

			////�߽������� �簢�� �׸���
			//rectangle(binary_frame, Rect(Point(kkr_pupilIndex.firstPupil_center.x - 8, kkr_pupilIndex.firstPupil_center.y - 7), Point(kkr_pupilIndex.firstPupil_center.x + 8, kkr_pupilIndex.firstPupil_center.y + 8)), Scalar(255, 255, 255));
			//rectangle(binary_frame, Rect(Point(kkr_pupilIndex.secondPupil_center.x - 8, kkr_pupilIndex.secondPupil_center.y - 7), Point(kkr_pupilIndex.secondPupil_center.x + 8, kkr_pupilIndex.secondPupil_center.y + 8)), Scalar(255, 255, 255));
			//�߽������� �簢�� �׸���
			rectangle(copy_frame, Rect(Point(kkr_mask.left_top.x + kkr_pupilIndex.firstPupil_center.x - 8, kkr_mask.left_top.y + kkr_pupilIndex.firstPupil_center.y - 7), Point(kkr_mask.left_top.x + kkr_pupilIndex.firstPupil_center.x + 8, kkr_mask.left_top.y + kkr_pupilIndex.firstPupil_center.y + 8)), Scalar(255, 255, 255));
			rectangle(copy_frame, Rect(Point(kkr_mask.left_top.x + kkr_pupilIndex.secondPupil_center.x - 8, kkr_mask.left_top.y + kkr_pupilIndex.secondPupil_center.y - 7), Point(kkr_mask.left_top.x + kkr_pupilIndex.secondPupil_center.x + 8, kkr_mask.left_top.y + kkr_pupilIndex.secondPupil_center.y + 8)), Scalar(255, 255, 255));

			//������ ��Ÿ���� index ���� ���� �簢�� �׸���
			//rectangle(binary_frame, Rect(Point(kkr_vIndex.first_pupil[0], kkr_hIndex.first_pupil[0]), Point(kkr_vIndex.first_pupil[1], kkr_hIndex.first_pupil[1])), Scalar(255, 255, 255));
			//rectangle(binary_frame, Rect(Point(kkr_vIndex.second_pupil[0], kkr_hIndex.second_pupil[0]), Point(kkr_vIndex.second_pupil[1], kkr_hIndex.second_pupil[1])), Scalar(255, 255, 255));

			if (kkr_stimulation.stimualtion == true)
			{
				circle(copy_frame, Point(300, kkr_stimulation.y), 1, Scalar(255, 0, 0), 2);
				kkr_stimulation.y++;
				if (kkr_stimulation.y >= copy_frame.rows)
				{
					kkr_stimulation.y = 0;
					kkr_stimulation.stimualtion = false;
				}
			}
			
			imshow("center", binary_frame);
			imshow("original_frame", copy_frame);
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
		else if (key == 'k')
		{
			kkr_stimulation.stimualtion = true;
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
		kkr_mask->left_top = { kkr_mask->center.x - kkr_mask->x_offset,kkr_mask->center.y - kkr_mask->y_offset };
		kkr_mask->right_bottom = { kkr_mask->center.x + kkr_mask->x_offset,kkr_mask->center.y + kkr_mask->y_offset };
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

//�����߽� ���� center = sum(��ǥ*��ǥ���� histogram value)/sum(histogram value)
void getFirstCenter(Mat& vProj, Mat& hProj, g_data* p)
{
	int numerator = 0;
	int denominator = 0;

	for (int i = p->firstPupil_leftTop.x; i <= p->firstPupil_rightBottom.x; i++)
	{
		numerator = numerator + vProj.at<uchar>(0, i)*i;
		denominator = denominator + vProj.at<uchar>(0, i);
	}

	// ������ ������ ������ ��� 0/0�� �Ǵ� ���� ���� ���� numerator!=0 && denominator!=0 ������ �ɾ��ش�.
	if (numerator != 0 && denominator != 0)
	{
		p->firstPupil_center.x = numerator / denominator;
		numerator = 0;
		denominator = 0;
	}

	for (int i = p->firstPupil_leftTop.y; i <= p->firstPupil_rightBottom.y; i++)
	{
		numerator = numerator + hProj.at<uchar>(0, i)*i;
		denominator = denominator + hProj.at<uchar>(0, i);
	}

	if (numerator != 0 && denominator != 0)
	{
		p->firstPupil_center.y = numerator / denominator;
		numerator = 0;
		denominator = 0;
	}
}

//�����߽� ���� center = sum(��ǥ*��ǥ���� histogram value)/sum(histogram value)
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