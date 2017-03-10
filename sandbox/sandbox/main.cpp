#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <list>
#include <algorithm>
#include <fstream>

#include <opencv2\opencv.hpp>

#include "kinect.h"
#include "ini.h"
#include "drag.h"

using namespace std;
using namespace cv;

#define WIDTH	(640)
#define HEIGHT	(480)
#define MAXS	(65535)
#define MAXB	(255)

template <class T>
cv::Mat hist(cv::Mat input, double max) {
	int i, j;
	double minVal, maxVal;
	cv::Mat output = input.clone();
	cv::minMaxLoc(input, &minVal, &maxVal, NULL, NULL);

	minVal = 65535;
	/*for (i = 0; i<output.cols; i++) {
		for (j = 0; j<output.rows; j++) {
			if (output.at<T>(j, i)>(T)0) {
				if (output.at<T>(j, i)<minVal) {
					minVal = output.at<T>(j, i);
				}
			}
		}
	}*/

	for (i = 0; i<output.cols; i++) {
		for (j = 0; j<output.rows; j++) {
			if (output.at<T>(j, i)>(T)0) {
				//output.at<T>(j,i)=max-(((output.at<T>(j,i)-minVal)/maxVal)*max);	// original
				output.at<T>(j, i) = (((output.at<T>(j, i) - minVal) / (maxVal - minVal))*max);	// test
			}
		}
	}
	return output;
}

double interpolate(double val, double y0, double x0, double y1, double x1) {
	return (val - x0)*(y1 - y0) / (x1 - x0) + y0;
}

double base(double val) {
	if (val <= -0.75) return 0;
	else if (val <= -0.25) return interpolate(val, 0.0, -0.75, 1.0, -0.25);
	else if (val <= 0.25) return 1.0;
	else if (val <= 0.75) return interpolate(val, 1.0, 0.25, 0.0, 0.75);
	else return 0.0;
}

double red(double gray) {
	return base(gray - 0.5);
}
double green(double gray) {
	return base(gray);
}
double blue(double gray) {
	return base(gray + 0.5);
}
uchar red(uchar gray) {
	return (uchar)(red(65535.0 / gray) * 255);
}
uchar green(uchar gray) {
	return (uchar)(green(65535.0 / gray) * 255);
}
uchar blue(uchar gray) {
	return (uchar)(blue(65535.0 / gray) * 255);
}
uchar red(uchar gray, double tr) {
	return (uchar)(red(tr / gray) * 255);
}
uchar green(uchar gray, double tr) {
	return (uchar)(green(tr / gray) * 255);
}
uchar blue(uchar gray, double tr) {
	return (uchar)(blue(tr / gray) * 255);
}

bool myfunction(int i, int j) { return (i<j); }

bool click = false;
point cur;
// mouse callback
void mouse(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		click = true;
		//cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
		cur.x = x;
		cur.y = y;
	}
	else if (event == EVENT_RBUTTONDOWN)
	{
		//cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
		cur.x = x;
		cur.y = y;
	}
	else if (event == EVENT_MBUTTONDOWN)
	{
		//cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
		cur.x = x;
		cur.y = y;
	}
	else if (event == EVENT_MOUSEMOVE)
	{
		//cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;
		cur.x = x;
		cur.y = y;
	}
	else if (event == EVENT_LBUTTONUP) {
		click = false;
		//cout << "Left button of the mouse is released - position (" << x << ", " << y << ")" << endl;
		cur.x = x;
		cur.y = y;
	}
}

string type2str(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}

int main(int argc, char* argv[]) {
	int s_max_dil = 0;
	int s_max_buf = 0;
	int s_init_dil = 0;
	int s_init_buf = 0;
	int s_w = 0;
	int s_h = 0;
	// read settings from file
	if (argc > 1) {
		es::ini Settings(argv[1]);
		s_max_dil = Settings.readi("MAX", "Dilate");
		s_max_buf = Settings.readi("MAX", "Buffer");
		s_init_dil = Settings.readi("INITIAL", "Dilate");
		s_init_buf = Settings.readi("INITIAL", "Buffer");
		s_w = Settings.readi("RESIZE", "w");
		s_h = Settings.readi("RESIZE", "h");
	}
	// if error in reading set defaults
	if (!s_max_dil)s_max_dil	= 20;
	if (!s_max_buf)s_max_buf	= 40;
	if (!s_init_dil)s_init_dil	= 7;
	if (!s_init_buf)s_init_buf	= 5;
	if (!s_w)s_w				= 1280;
	if (!s_h)s_h				= 960;

	bool exitapp = false;
	es::kinect kin;
	//Mat frame=Mat::zeros(480, 640, CV_16UC3);
	int iHist = 28000;
	int iROI = HEIGHT / 2;
	int iDist = 1;
	int iRange = 1;
	signed int iX = WIDTH/2;
	signed int iY = HEIGHT/2;
	unsigned short usFrom = 0;
	unsigned short usTo = MAXS;
	int iDilat = s_init_dil;
	int iBuf = s_init_buf;
	int iFrameCount = 0;
	list<Mat> buffer;
	list<Mat>::iterator it;
	int iFlip = 3;
	int iLogo = 0;
	int iLogoLevel = 0;
	int iLevelMax = 360;
	int bCanny = 1;

	if (!kin.init()) {
		cout << "Kinect: Unable to connect!" << endl;
		return -1;
	}


	namedWindow("Control", WINDOW_NORMAL);
	namedWindow("Sandbox", WINDOW_NORMAL);
	createTrackbar("Histogram", "Control", &iHist, MAXS);
	//createTrackbar("FoV", "Control", &iROI, (HEIGHT / 2));
	//createTrackbar("Distance", "Control", &iDist, 20);
	//createTrackbar("Range", "Control", &iRange, 20);
	createTrackbar("Dilate", "Control", &iDilat, s_max_dil);
	createTrackbar("Buffer", "Control", &iBuf, s_max_buf);
	createTrackbar("Flip", "Control", &iFlip, 3);
	createTrackbar("Vrstevnice", "Control", &bCanny, 1);
	createTrackbar("logo", "Control", &iLogo, 2);
	createTrackbar("logoLevel", "Control", &iLogoLevel, 99);
	//createTrackbar("LevelMAX", "Control", &iLevelMax, 360);
	setMouseCallback("Control", mouse, NULL);

	drag cResize(rect(0, 0, WIDTH, HEIGHT), rect(0, 0, WIDTH, HEIGHT));

	// mouse states
	int iOldBufferSize = 0;
	cur.x = 0;
	cur.y = 0;
	rect area=rect(0, 0, WIDTH, HEIGHT);

	Mat logo = imread("logo.png");

	while (!exitapp) {
		kin.update();
		Mat tmp, tmpor, edge;
		Mat fin(tmp.rows, tmp.cols, CV_8UC3), pixelIN(1, 1, CV_8UC3), pixelOUT(1, 1, CV_8UC3);
		Mat test, work, work2;

		Mat raw(HEIGHT, WIDTH, CV_16UC1, kin.getDepth16());
		Mat raw2 = hist<unsigned short>(raw, (double)iHist); //65535.0f
		
		if (cResize.update(cur, click)) { // update coordinates
			area = cResize.getActiveArea(); // get current dragging area
			rectangle(raw2, Rect(area.left, area.top, area.right - area.left, area.bottom - area.top), Scalar(65535), 1);
			buffer.clear();
		}
		else {
			tmpor = raw2(Rect(area.left, area.top, area.right - area.left, area.bottom - area.top));
			if (iOldBufferSize != iBuf)
				buffer.clear();
			if (iBuf > 0)
				buffer.push_front(tmpor);
			if (buffer.size() == iBuf && iBuf > 0) {
				it = buffer.begin();
				// Create a 0 initialized image to use as accumulator
				Mat m(it->rows, it->cols, CV_64FC1);
				m.setTo(Scalar(0, 0, 0, 0));
				// Use a temp image to hold the conversion of each input image to CV_64FC3
				// This will be allocated just the first time, since all your images have
				// the same size.
				Mat temp;
				for (it = buffer.begin(); it != buffer.end(); ++it)
				{
					// Convert the input images to CV_64FC3 ...
					it->convertTo(temp, CV_64FC1);
					// ... so you can accumulate
					m += temp;
				}
				// Convert back to CV_8UC3 type, applying the division to get the actual mean
				m.convertTo(tmpor, CV_16UC1, 1. / buffer.size());
				buffer.pop_back();
			}
			iOldBufferSize = iBuf;

			tmpor.convertTo(tmp, CV_8UC1, ((double)0.00390625)); //0.00390625
			//if (tmp.cols > 10 && tmp.rows > 10)
			//	imshow("temporaly", tmp);
			
			if (iFlip < 3) {
				flip(tmp, tmp, ((signed int)iFlip) - 1);
			}

			bitwise_not(tmp, tmp);
			equalizeHist(tmp, tmp);
			cvtColor(tmp, work2, CV_GRAY2BGR);
			work2.convertTo(work, CV_8UC3);
			Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * iDilat + 1, 2 * iDilat + 1), Point(iDilat, iDilat));
			dilate(work, work, element);
			applyColorMap(work, work, COLORMAP_JET);
			if (bCanny) {
				Canny(work, edge, iDist, iRange);
				//cout << "canny " << type2str(edge.type()) << endl;
				cvtColor(edge, test, CV_GRAY2BGR);
				test.convertTo(test, CV_8UC3);
				//equalizeHist(test, test);
				if (CV_8UC3 != test.type())
					cout << "zle";
				//addWeighted(work, 0.5, test, 0.5, 0.0, fin);
				work.copyTo(fin);
				fin.setTo(Scalar(255, 255, 255), edge);
			}
			else {
				work.copyTo(fin);
			}
			if (iLogo > 0) {
				Mat logoMask;
				if (iLogo == 1) {
					logoMask = Mat::ones(logo.size(), CV_8U);
				}
				if (iLogo == 2) {
					threshold(logo, logoMask, 10, 255, 0);
				}
				Mat maskedLogo, maskedFin;
				logo.copyTo(maskedLogo, logoMask);
				fin(cv::Rect(0, 0, logoMask.cols, logoMask.rows)).copyTo(maskedFin, logoMask);
				Mat smallFin;
				addWeighted(maskedLogo, (iLogoLevel / 100.0), maskedFin, ((99 - iLogoLevel) / 100.0), 0.0, smallFin);
				Mat bigmask = Mat::zeros(fin.rows, fin.cols, logoMask.type());
				logoMask.copyTo(bigmask(cv::Rect(0, 0, logoMask.cols, logoMask.rows)));
				smallFin.copyTo(fin(cv::Rect(0, 0, smallFin.cols, smallFin.rows)), logoMask);
			}
		}
		/*for (int i = 0; i < tmp.rows; i++) {
			for (int j = 0; j < tmp.cols; j++) {
				double depth = tmp.at<ushort>(i, j);
				double newMax = iDist + (iRange / 2.0);
				double newMin = iDist - (iRange / 2.0);
				int stepD = ((int)iRange+(int)newMin) / iLevel;
				int stepH = iLevelMax / iLevel;
				int hue = stepH*(((int)depth + (int)newMin) / stepD);
				if (hue == 0) {
					pixelIN.at<Vec3b>(0, 0) = Vec3b((uchar)hue, 200, 0);
				}
				else {
					pixelIN.at<Vec3b>(0, 0) = Vec3b((uchar)hue, 255, 255);
				}
				cvtColor(pixelIN, pixelOUT, CV_HSV2BGR);
				fin.at<Vec3b>(i, j) = pixelOUT.at<Vec3b>(0, 0);
			}
		}*/

		
		//if (edge.cols > 10 && edge.rows > 10)
		//	imshow("edge", edge);
		if (raw2.cols > 10 && raw2.rows > 10)
			imshow("Control", raw2);
		if (fin.cols > 0 && fin.rows > 0) {
			imshow("Sandbox", fin);
		}
		
		/*
		int xfrom = (WIDTH / 2) - iROI;
		int xto = (WIDTH / 2) + iROI;
		int yfrom = (HEIGHT / 2) - iROI;
		int yto = (HEIGHT / 2) + iROI;
		if (xfrom < 0)
			xfrom = 0;
		if (yfrom < 0)
			yfrom = 0;
		if (xto > WIDTH)
			xto = WIDTH;
		if (yto > HEIGHT)
			yto = HEIGHT;
		Mat roi = raw2(Range(yfrom, yto), Range(xfrom, xto));

		if (iFlip < 3) {
			flip(roi, roi, ((signed int)iFlip)-1);
		}

		buffer.push_front(roi);
		if (iBuf < 3)
			iBuf = 3;
		if (buffer.size() > iBuf) {

			Mat frame = Mat::zeros(Size(roi.cols, roi.rows), CV_16UC1);

			for (int i = 0; i < roi.cols; i++) {
				for (int j = 0; j < roi.rows; j++) {
					vector<ushort> input;
					for (it = buffer.begin(); it != buffer.end(); ++it) {
						input.push_back(it->at<ushort>(j,i));
					}
					sort(input.begin(), input.end(), myfunction);
					frame.at<ushort>(j, i) = input[(iBuf / 2)];
					input.empty();
				}
			}

			

			Mat fin = Mat::zeros(Size(frame.cols, frame.rows), CV_8UC3);
			usFrom = iDist - (iRange / 2);
			usTo = iDist + (iRange / 2);
			if (usFrom < 0)
				usFrom = 0;
			if (usTo > MAXS)
				usTo = MAXS;
			for (int i = 0; i < fin.cols; i++) {
				for (int j = 0; j < fin.rows; j++) {
					ushort inten = frame.at<ushort>(j, i);
					if ((inten > usFrom) && (inten < usTo)) {
						inten = (uchar)((((double)usTo - (double)usFrom) / ((double)inten - (double)usFrom))*(double)MAXB);
					}
					else {
						inten = (uchar)0;
					}
					fin.at<Vec3b>(j, i) = Vec3b(inten, inten, inten);
				}
			}


			resize(fin, fin, Size(s_w, s_h));
			Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * iDilat + 1, 2 * iDilat + 1), Point(iDilat, iDilat));
			dilate(fin, fin, element);
			applyColorMap(fin, fin, COLORMAP_JET);
			
			imshow("Raw", raw2);
			imshow("Control", fin);
			imshow("Sandbox", fin);

			buffer.pop_back();
		}
		*/
		if (GetAsyncKeyState(VK_HOME) & 0x8000) {
			// save fin as RAW
			cout << "saving...\n";
			resize(tmp, tmp, Size(200, 200));
			imshow("temp", tmp);

			int rows = tmp.rows;      // 880)
			int cols = tmp.cols;      // 720
			uchar* buffer = tmp.data; // 8-bit pixel

										// Write frame data into raw file
			string filename("hightmap.raw");
			std::ofstream outfile(filename.c_str(), ios::out | ios::binary);
			outfile.write((char*)(buffer), tmp.total());  // In byte so frame.total() should be enough ?
			outfile.close();

		}
		if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
			area = rect(0, 0, WIDTH, HEIGHT);
			buffer.clear();
		}
		if (GetAsyncKeyState(VK_END) & 0x8000) {
			buffer.clear();
		}
		waitKey(1);
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			exitapp = true;
	}

	kin.term();
	return 0;
}