#include <opencv2\opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include <stdio.h>
#include <Windows.h>

using namespace cv;
using namespace std;

bool edge = false;
bool gaus = false;
bool medi = false;
bool bgst = false;
bool bgss = false;

int main(int, char**)
{
	Mat frame, edges, fin, a, b;
	int tre1 = 100, tre2 = 200, trbl=1;

	Mat fgMaskMOG2;
	Ptr<BackgroundSubtractor> pMOG2;
	Mat bgblur;
	Mat bg;
	Mat blabla;
	Mat ero;
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5), Point(2, 2));
	
	//--- INITIALIZE VIDEOCAPTURE
	VideoCapture cap;
	// open the default camera using default API
	cap.open(0);
	// OR advance usage: select any API backend
	int deviceID = 0;             // 0 = open default camera
	int apiID = cv::CAP_ANY;      // 0 = autodetect default API
								  // open selected camera using selected API

	cap.open(deviceID + apiID);
	// check if we succeeded
	if (!cap.isOpened()) {
		cerr << "ERROR! Unable to open camera\n";
		return -1;
	}

	namedWindow("ATS", 1);
	createTrackbar("tr1", "ATS", &tre1, 1000);
	createTrackbar("tr2", "ATS", &tre2, 1000);
	createTrackbar("blur", "ATS", &trbl, 30);

	pMOG2 = createBackgroundSubtractorMOG2(500, 16.0, false);
	

	//--- GRAB AND WRITE LOOP
	cout << "Start grabbing" << endl
		<< "Press any key to terminate" << endl;
	for (;;)
	{
		// wait for a new frame from camera and store it into 'frame'
		cap.read(frame);
		// check if we succeeded
		if (frame.empty()) {
			cerr << "ERROR! blank frame grabbed\n";
			break;
		}
		
		// keyboard
		if (GetAsyncKeyState(0x45)) {
			if (edge)
				edge = false;
			else
				edge = true;
		}

		if (GetAsyncKeyState(0x47)) {
			if (gaus)
				gaus = false;
			else
				gaus = true;
		}
		if (GetAsyncKeyState(0x4D)) {
			if (medi)
				medi = false;
			else
				medi = true;
		}
		if (GetAsyncKeyState(0x42)) {
			if (bgst)
				bgst = false;
			else
				bgst = true;
		}
		if (GetAsyncKeyState(0x53)) {
			if (bgss)
				bgss = false;
			else
				bgss = true;
		}

		//processing
		if (bgst) {
			GaussianBlur(frame, bgblur, Size(9, 9), 0, 0);
			pMOG2->apply(bgblur, fgMaskMOG2);
			erode(fgMaskMOG2, ero, element);
			Mat red = Mat(frame.size(), frame.type(), Vec3b(0x0, 0x0, 0x255));
			Mat gg;
			red.copyTo(gg, ero);
			//threshold(gg, gg, 1.0, 255.0, THRESH_BINARY);
			//imshow("ero", gg);
			addWeighted(frame, 0.5, gg, 0.5, 0, frame);

			/*for (int i = 0; i < frame.rows; i++) {
				for (int j = 0; j < frame.cols; j++) {
					if (fgMaskMOG2.at<unsigned char>(i, j) > 0x00) {
						unsigned char tmp = frame.at<Vec3b>(i, j)[1];
						frame.at<Vec3b>(i, j)[0] = 0x255;
						frame.at<Vec3b>(i, j)[1] = frame.at<Vec3b>(i, j)[2];
						frame.at<Vec3b>(i, j)[2] = tmp;
					}
				}
			}*/
			
		}
		if (bgss) {
			if (GetAsyncKeyState(VK_SNAPSHOT)) {
				frame.copyTo(bg);
				GaussianBlur(bg, bg, Size(9, 9), 0, 0);
				cout << "New snapshot." << endl;
			}
			Mat blure;
			frame.copyTo(blure);
			GaussianBlur(blure, blure, Size(9, 9), 0, 0);
			for (int i = 0; i < frame.rows; i++) {
				for (int j = 0; j < frame.cols; j++) {
					if (10.0 < norm(bg.at<Vec3b>(i, j), frame.at<Vec3b>(i, j), NORM_L2)) {
						unsigned char tmp = frame.at<Vec3b>(i, j)[1];
						frame.at<Vec3b>(i, j)[0] = 0x255;
						frame.at<Vec3b>(i, j)[1] = frame.at<Vec3b>(i, j)[2];
						frame.at<Vec3b>(i, j)[2] = tmp;
					}
				}
			}
		}
		else {
			frame.copyTo(bg);
			GaussianBlur(bg, bg, Size(9, 9), 0, 0);
		}
		if(medi) {
			trbl = ((trbl / 2) * 2) + 1;
			medianBlur(frame, frame, trbl);
		}
		if (gaus) {
			trbl = ((trbl / 2) * 2) + 1;
			GaussianBlur(frame, frame, Size(trbl, trbl), 0, 0);
		}

		if (edge) {
			Canny(frame, edges, tre1, tre2);
			edges.convertTo(edges, CV_8UC1);
			frame.convertTo(frame, CV_8UC3);

			for (int i = 0; i < frame.rows; i++) {
				for (int j = 0; j < frame.cols; j++) {
					if (edges.at<unsigned char>(i, j) > 0x00) {
						frame.at<Vec3b>(i, j)[0] = 0x00;
						frame.at<Vec3b>(i, j)[1] = 0x00;
						frame.at<Vec3b>(i, j)[2] = 0x255;
					}
				}
			}
		}

		resize(frame, fin, Size(1600, 1200));

		line(fin, Point(0, 600), Point(1599, 600), Scalar(255, 255, 255));
		line(fin, Point(800, 0), Point(800, 1199), Scalar(255, 255, 255));
		ellipse(fin, Point(800, 600), Size(500, 500), 0.0f, 0.0f, 360.0f, Scalar(255, 255, 255));
		putText(fin, "ADVANCED TARGETING SYSTEM", Point(10, 50), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 255, 0));
		if (edge)
			putText(fin, "E - EDGE SYSTEM: ON", Point(10, 100), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 255, 0));
		else
			putText(fin, "E - EDGE SYSTEM: OFF", Point(10, 100), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 255, 0));

		if (gaus)
			putText(fin, "G - GAUS SYSTEM: ON", Point(10, 150), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 255, 0));
		else
			putText(fin, "G - GAUS SYSTEM: OFF", Point(10, 150), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 255, 0));

		if (medi)
			putText(fin, "M - MEDI SYSTEM: ON", Point(10, 200), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 255, 0));
		else
			putText(fin, "M - MEDI SYSTEM: OFF", Point(10, 200), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 255, 0));

		if (bgst)
			putText(fin, "B - BGST SYSTEM: ON", Point(10, 250), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 255, 0));
		else
			putText(fin, "B - BGST SYSTEM: OFF", Point(10, 250), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 255, 0));

		if (bgss)
			putText(fin, "S - BGSS SYSTEM: ON", Point(10, 300), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 255, 0));
		else
			putText(fin, "S - BGSS SYSTEM: OFF", Point(10, 300), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0, 255, 0));

		// show live and wait for a key with timeout long enough to show images
		imshow("ATS", fin);
		//imshow("Esge", edges);
		if (waitKey(5) == 27)
			break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}