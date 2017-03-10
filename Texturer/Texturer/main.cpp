#include <iostream>
#include <stdio.h>
#include <list>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <thread>

#include <Windows.h>

#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

bool click = false;
POINT cur;

void mouse(int event, int xx, int yy, int flags, void* userdata)
{
	int x = (LONG)xx;
	int y = (LONG)yy;

	if (event == EVENT_LBUTTONDOWN)
	{
		click = true;
#ifdef DEBUG
		cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
#endif
		cur.x = x;
		cur.y = y;
	}
	else if (event == EVENT_RBUTTONDOWN)
	{
#ifdef DEBUG
		cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
#endif
		cur.x = x;
		cur.y = y;
	}
	else if (event == EVENT_MBUTTONDOWN)
	{
#ifdef DEBUG
		cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
#endif
		cur.x = x;
		cur.y = y;
	}
	else if (event == EVENT_MOUSEMOVE)
	{
#ifdef DEBUG
		cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;
#endif
		cur.x = x;
		cur.y = y;
	}
	else if (event == EVENT_LBUTTONUP) {
		click = false;
#ifdef DEBUG
		cout << "Left button of the mouse is released - position (" << x << ", " << y << ")" << endl;
#endif
		cur.x = x;
		cur.y = y;
	}
}

HWND GetConsoleHwnd(void)
{
#define MY_BUFSIZE 1024 // Buffer size for console window titles.
	HWND hwndFound;         // This is what is returned to the caller.
	char pszNewWindowTitle[MY_BUFSIZE]; // Contains fabricated
										// WindowTitle.
	char pszOldWindowTitle[MY_BUFSIZE]; // Contains original
										// WindowTitle.

										// Fetch current window title.

	GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);

	// Format a "unique" NewWindowTitle.

	wsprintf(pszNewWindowTitle, "%d/%d",
		GetTickCount(),
		GetCurrentProcessId());

	// Change current window title.

	SetConsoleTitle(pszNewWindowTitle);

	// Ensure window title has been updated.

	Sleep(40);

	// Look for NewWindowTitle.

	hwndFound = FindWindow(NULL, pszNewWindowTitle);

	// Restore original window title.

	SetConsoleTitle(pszOldWindowTitle);

	return(hwndFound);
}
string OpenFile() {
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
	HWND hwnd;              // owner window
	HANDLE hf;              // file handle
	hwnd = GetConsoleHwnd();
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	// Display the Open dialog box. 
	if (GetOpenFileName(&ofn) == TRUE)
		/*hf = CreateFile(ofn.lpstrFile,
			GENERIC_READ,
			0,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL);*/
		return string(szFile);
	else
		return string("");
}
string SaveFile() {
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
	HWND hwnd;              // owner window
	HANDLE hf;              // file handle
	hwnd = GetConsoleHwnd();
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;
	// Display the Open dialog box. 
	if (GetSaveFileName(&ofn) == TRUE)
		/*hf = CreateFile(ofn.lpstrFile,
		GENERIC_READ,
		0,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);*/
		return string(szFile);
	else
		return string("");
}


int main() {
	int w = 1, h = 1;
	bool exitapp = false;
	string path;
	Mat backBuffer = Mat::zeros(Size(300, 100), CV_8UC3);
	Mat input, working;
	bool mode = false;

	namedWindow("Texturer", WINDOW_AUTOSIZE | CV_GUI_NORMAL);
	namedWindow("Preview", WINDOW_AUTOSIZE);
	createTrackbar("width", "Texturer", &w, 100);
	createTrackbar("height", "Texturer", &h, 100);
	setMouseCallback("Texturer", mouse, NULL);


	while (!exitapp) {
		// openfile
		if (click) {
			if (cur.x <= 99 &&
				cur.x >= 0 &&
				cur.y >= 0 &&
				cur.y <= 49) {
				path = OpenFile(); 
				input = imread(path);
				click = false;
			}
		}
		// savefile
		if (click) {
			if (cur.x <= 199 &&
				cur.x >= 100 &&
				cur.y >= 0 &&
				cur.y <= 49) {
				path = SaveFile();
				imwrite(path, working);
				click = false;
			}
		}
		// exit
		if (click) {
			if (cur.x <= 299 &&
				cur.x >= 200 &&
				cur.y >= 0 &&
				cur.y <= 49) {
				exitapp = true;
			}
		}
		// flipping
		if (click) {
			if (cur.x <= 149 &&
				cur.x >= 0 &&
				cur.y >= 50 &&
				cur.y <= 99) {
				mode = true;
			}
		}
		// copying
		if (click) {
			if (cur.x <= 299 &&
				cur.x >= 150 &&
				cur.y >= 50 &&
				cur.y <= 99) {
				mode = false;
			}
		}

		if (input.cols < 1 && input.rows < 1)
			working = Mat::zeros(100, 100, CV_8UC3);
		else {
			if (w < 1)w = 1; if (h < 1)h = 1;
			working = Mat::zeros(h*input.rows, w*input.cols, CV_8UC3);
			//working(Rect(0, 0, input.cols, input.rows)) = input;
			input.copyTo(working(Rect(0, 0, input.cols, input.rows)));
			for (int i = 0; i < w; i++) {
				for (int j = 0; j < h; j++) {
					if (mode) {
						Rect from, to;
						from.x = 0;
						from.y = 0;
						from.width = input.cols;
						from.height = input.rows;
						to.x = (input.cols*i);
						to.y = (input.rows*j);
						to.width = input.cols;
						to.height = input.rows;
						Mat temp, temp2;
						input(from).copyTo(temp);
						if (((i % 2) == 0) && ((j % 2) == 0)) {
							temp2 = temp;
						}
						if (((i % 2) == 0) && ((j % 2) == 1)) {
							flip(temp, temp2, 0);
						}
						if (((i % 2) == 1) && ((j % 2) == 0)) {
							flip(temp, temp2, 1);
						}
						if (((i % 2) == 1) && ((j % 2) == 1)) {
							flip(temp, temp2, -1);
						}
						temp2.copyTo(working(to));
					}
					else {
						Rect from, to;
						from.x = 0;
						from.y = 0;
						from.width = input.cols;
						from.height = input.rows;
						to.x = (input.cols*i);
						to.y = (input.rows*j);
						to.width = input.cols;
						to.height = input.rows;
						input(from).copyTo(working(to));
					}
				}
			}
		}

		rectangle(backBuffer, Rect(0,   0, 100, 50), Scalar(255, 0, 0), CV_FILLED);
		rectangle(backBuffer, Rect(100, 0, 100, 50), Scalar(0, 155, 0), CV_FILLED);
		rectangle(backBuffer, Rect(200, 0, 100, 50), Scalar(0, 0, 255), CV_FILLED);
		rectangle(backBuffer, Rect(0,   0, 100, 50), Scalar(255, 255, 255), 1);
		rectangle(backBuffer, Rect(100, 0, 100, 50), Scalar(255, 255, 255), 1);
		rectangle(backBuffer, Rect(200, 0, 100, 50), Scalar(255, 255, 255), 1);
		putText(backBuffer, "Open", Point(10, 40), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 3);
		putText(backBuffer, "Save", Point(110, 40), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 3);
		putText(backBuffer, "Exit", Point(210, 40), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 3);

		if (mode) {
			rectangle(backBuffer, Rect(0, 50, 150, 50), Scalar(100, 155, 100), CV_FILLED);
			rectangle(backBuffer, Rect(150, 50, 150, 50), Scalar(50, 50, 50), CV_FILLED);
		}
		else {
			rectangle(backBuffer, Rect(0, 50, 150, 50), Scalar(50, 50, 50), CV_FILLED);
			rectangle(backBuffer, Rect(150, 50, 150, 50), Scalar(100, 155, 100), CV_FILLED);
		}
		rectangle(backBuffer, Rect(0, 50, 150, 50), Scalar(255, 255, 255), 1);
		rectangle(backBuffer, Rect(150, 50, 150, 50), Scalar(255, 255, 255), 1);
		putText(backBuffer, "Flipping", Point(10, 90), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 3);
		putText(backBuffer, "Copying", Point(160, 90), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 3);
		
		imshow("Texturer", backBuffer);
		imshow("Preview", working);
		waitKey(1);
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			exitapp = true;
	}
	return 0;
}