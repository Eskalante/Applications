//#ifndef ESKINECT
#pragma once
#define ESKINECT

/*
inc dir $(KINECTSDK10_DIR)\inc
lib dir $(WindowsSdkDir)lib
lib dir $(KINECTSDK10_DIR)\lib\x86
lib in  Kinect10.lib
*/

#pragma comment (lib, "Kinect10.lib")

#include <Windows.h>
#include "NuiApi.h"
#include <limits>

namespace es {

	// CONST
	// skeleton
	const float zoom = 5.0f;		// virtual zoom of kinect
	const float x_offset = 0.0f;	// x offset of central asix
	const float y_offset = 0.0f;// y -//-   -500.0f
	enum PART {					// part of sceleton to process
		hand_l = NUI_SKELETON_POSITION_HAND_LEFT,
		hand_r = NUI_SKELETON_POSITION_HAND_RIGHT,
		body = NUI_SKELETON_POSITION_SHOULDER_CENTER
	};
	enum COORDINATES {			// format of coordinates of returned sceleton part
		absolute_cm,			// actual position of sceleton part from kinects central asix
		relative_cm,			// shift from previous position
		absolute_px,			// -//- in px by windows width and height
		relative_px				// -//- -//-
	};
	// depth
	static const int        cDepthWidth = 640;
	static const int        cDepthHeight = 480;
	static const int        cBytesPerPixel = 4;//4
											   // other
	const int	eventCount = 1;	// num of events handling kinect processes


								// STRUCT
	typedef struct CRDt {		// coordinates
		union {
			float			fx;	// for cm
			signed int		ix;	// for px
		};
		union {
			float			fy;	// for cm
			signed int		iy;	// for px
		};
		union {
			float			fz;	// for cm
			signed int		iz;	// for px
		};
	}CRD;
	typedef struct POINT3t {		// 3D point
		float x;
		float y;
		float z;
		float w;				// tmp
	}POINT3;

	// KINECT
	class kinect {

		// C&D
	public:
		kinect() {}
		~kinect() {}

		// SYS
	public:
		bool init() {
			m_depthRGBX = new BYTE[cDepthWidth*cDepthHeight*cBytesPerPixel];
			m_depth = new USHORT[cDepthWidth*cDepthHeight];

			for (register int i = 0; i<NUI_SKELETON_POSITION_COUNT; i++) {
				m_Points_old[i].x = 0.0f;
				m_Points_old[i].y = 0.0f;
				m_Points_old[i].z = 0.0f;
				m_Points_old[i].w = 0.0f;
				m_Points[i].x = 0.0f;
				m_Points[i].y = 0.0f;
				m_Points[i].z = 0.0f;
				m_Points[i].w = 0.0f;
			}

			m_hNextDepthFrameEvent = INVALID_HANDLE_VALUE;
			m_pDepthStreamHandle = INVALID_HANDLE_VALUE;
			m_hNextSkeletonEvent = INVALID_HANDLE_VALUE;
			m_pSkeletonStreamHandle = INVALID_HANDLE_VALUE;
			ZeroMemory(m_Points, sizeof(m_Points));

			INuiSensor * pNuiSensor;
			int iSensorCount = 0;
			HRESULT hr = NuiGetSensorCount(&iSensorCount);

			if (FAILED(hr)) {
				return false;
			}

			// Look at each Kinect sensor
			for (int i = 0; i < iSensorCount; ++i) {
				// Create the sensor so we can check status, if we can't create it, move on to the next
				hr = NuiCreateSensorByIndex(i, &pNuiSensor);
				if (FAILED(hr)) {
					continue;
				}
				// Get the status of the sensor, and if connected, then we can initialize it
				hr = pNuiSensor->NuiStatus();
				if (S_OK == hr) {
					m_pNuiSensor = pNuiSensor;
					break;
				}
				// This sensor wasn't OK, so release it since we're not using it
				pNuiSensor->Release();
			}

			if (NULL != m_pNuiSensor)
			{
				// Initialize the Kinect and specify that we'll be using skeleton
				hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_DEPTH);
				//hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH);
				if (SUCCEEDED(hr)) {
					// Create an event that will be signaled when skeleton data is available
					m_hNextSkeletonEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
					m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
					// Open a skeleton stream to receive skeleton data
					hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0);
					hr = m_pNuiSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_640x480, 0, 2, m_hNextDepthFrameEvent, &m_pDepthStreamHandle);
				}
			}
			hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT);
			hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE);


			if (NULL == m_pNuiSensor || FAILED(hr)) {
				return false;
			}

			return true;
		}
		void term() {
			if (m_pNuiSensor) {
				m_pNuiSensor->NuiShutdown();
				m_pNuiSensor->Release();
				m_pNuiSensor = NULL;
			}
			if (m_hNextSkeletonEvent && (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE)) {
				CloseHandle(m_hNextSkeletonEvent);
			}
			if (m_hNextDepthFrameEvent && (m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE)) {
				CloseHandle(m_hNextDepthFrameEvent);
			}
			//SafeRelease(m_pNuiSensor);
			delete[] m_depth;
			delete[] m_depthRGBX;
		}
		void update() {
			hEvents[0] = m_hNextSkeletonEvent;
			//MsgWaitForMultipleObjects(eventCount, hEvents, FALSE, INFINITE, QS_ALLINPUT);

			if (NULL == m_pNuiSensor) {
				return;
			}
			// Wait for 0ms, just quickly test if it is time to process a skeleton
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonEvent, 0)) {
				ProcessSkeleton();
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextDepthFrameEvent, 0)) {
				ProcessDepth();
			}
		}

		// ACC
	public:
		CRD getPos(PART part, COORDINATES cor, int width = 0, int height = 0) {
			CRD s;
			if (cor == absolute_cm) {
				s.fx = m_Points[part].x;
				s.fy = m_Points[part].y;
				s.fz = m_Points[part].z;
			}
			if (cor == relative_cm) {
				s.fx = m_Points[part].x - m_Points_old[part].x;
				s.fy = m_Points[part].y - m_Points_old[part].y;
				s.fz = m_Points[part].z - m_Points_old[part].z;
			}
			if (cor == absolute_px) {
				POINT3 p = SkeletonToScreen(m_Points[part], width, height);
				s.ix = (signed int)p.x;
				s.iy = (signed int)p.y;
				s.iz = (signed int)p.z;
			}
			if (cor == relative_px) {
				POINT3 in;
				in.x = m_Points[part].x - m_Points_old[part].x;
				in.y = m_Points[part].y - m_Points_old[part].y;
				in.z = m_Points[part].z - m_Points_old[part].z;
				in.w = m_Points[part].w - m_Points_old[part].w;
				POINT3 p = SkeletonToScreen(in, width, height);
				s.ix = (signed int)p.x;
				s.iy = (signed int)p.y;
				s.iz = (signed int)p.z;
			}
			return s;
		}
		BYTE* getDepth() const {
			return m_depthRGBX;
		}
		USHORT* getDepth16() const {
			return m_depth;
		}

		// ATR
	private:
		// Current Kinect
		INuiSensor*	m_pNuiSensor;
		// Skeleton
		POINT3		m_Points[NUI_SKELETON_POSITION_COUNT];
		POINT3		m_Points_old[NUI_SKELETON_POSITION_COUNT];
		HANDLE		m_pSkeletonStreamHandle;
		HANDLE		m_hNextSkeletonEvent;
		// depth
		BYTE*		m_depthRGBX;
		USHORT*		m_depth;
		HANDLE		m_pDepthStreamHandle;
		HANDLE		m_hNextDepthFrameEvent;
		// tmp
		HANDLE		hEvents[eventCount];

		// HELP
	private:
		template<class Interface>
		inline void SafeRelease(Interface *& pInterfaceToRelease)
		{
			if (pInterfaceToRelease != NULL)
			{
				pInterfaceToRelease->Release();
				pInterfaceToRelease = NULL;
			}
		}
		void ProcessSkeleton() {
			NUI_SKELETON_FRAME skeletonFrame = { 0 };
			HRESULT hr = m_pNuiSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
			int i = 0;
			if (FAILED(hr)) {
				return;
			}
			// smooth out the skeleton data
			m_pNuiSensor->NuiTransformSmooth(&skeletonFrame, NULL);

			for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
				NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame.SkeletonData[i].eTrackingState;
				if (NUI_SKELETON_TRACKED == trackingState) {
					// We're tracking the skeleton, process it
					for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; ++j) {
						m_Points_old[j].x = m_Points[j].x;
						m_Points_old[j].y = m_Points[j].y;
						m_Points_old[j].z = m_Points[j].z;
						m_Points_old[j].w = m_Points[j].w;
						m_Points[j].x = skeletonFrame.SkeletonData[i].SkeletonPositions[j].x;
						m_Points[j].y = skeletonFrame.SkeletonData[i].SkeletonPositions[j].y;
						m_Points[j].z = skeletonFrame.SkeletonData[i].SkeletonPositions[j].z;
						m_Points[j].w = skeletonFrame.SkeletonData[i].SkeletonPositions[j].w;
					}
				}
				else if (NUI_SKELETON_POSITION_ONLY == trackingState) {
					// we've only received the center point of the skeleton, draw that
					;
				}
			}


		}
		void ProcessDepth() {
			HRESULT hr;
			NUI_IMAGE_FRAME imageFrame;

			// Attempt to get the depth frame
			hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);
			if (FAILED(hr)) {
				return;
			}

			INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
			NUI_LOCKED_RECT LockedRect;

			// Lock the frame data so the Kinect knows not to modify it while we're reading it
			pTexture->LockRect(0, &LockedRect, NULL, 0);

			// Make sure we've received valid data
			if (LockedRect.Pitch != 0) {
				int minDepth = (NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
				int maxDepth = (NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

				BYTE * rgbrun = m_depthRGBX;//BYTE*
				USHORT * drun = m_depth;// depth
				const USHORT * pBufferRun = (const USHORT *)LockedRect.pBits;


				// end pixel is start + width*height - 1
				const USHORT * pBufferEnd = pBufferRun + (cDepthWidth * cDepthHeight);

				while (pBufferRun < pBufferEnd)
				{
					// discard the portion of the depth that contains only the player index
					USHORT depth = NuiDepthPixelToDepth(*pBufferRun);

					//test disparity
					if (depth == 0) {
						*(drun++) = (USHORT)0;
					}
					else {
						*(drun++) = (USHORT)(((USHORT)65535) - depth);
						/*if (depth >= 2000) {
							*(drun++) = (USHORT)0;
						}
						else {
							*(drun++) = (USHORT)(((USHORT)65535) - depth);
						}*/
					}

					//original depth
					//*(drun++)=depth;

					//if (depth >= 2000)//1500
					//	depth = 0;

					// to convert to a byte we're looking at only the lower 8 bits
					// by discarding the most significant rather than least significant data
					// we're preserving detail, although the intensity will "wrap"
					//BYTE intensity = static_cast<BYTE>(depth >= minDepth && depth <= maxDepth ? depth % 256 : 0);
					BYTE intensity = static_cast<BYTE>(depth % 1024);
					//BYTE intensity = static_cast<BYTE>(depth % 1024); /8

					if (intensity) {
						*(rgbrun++) = 255 - intensity;
						*(rgbrun++) = 255 - intensity;
						*(rgbrun++) = 255 - intensity;
					}
					else {
						*(rgbrun++) = (BYTE)intensity;
						*(rgbrun++) = (BYTE)intensity;
						*(rgbrun++) = (BYTE)intensity;
					}

					// We're outputting BGR, the last byte in the 32 bits is unused so skip it
					// If we were outputting BGRA, we would write alpha here.
					//++rgbrun;
					*(rgbrun++) = 0;

					// Increment our index into the Kinect's depth buffer
					++pBufferRun;
				}

				;// Draw the data with Direct2D
				 //m_pDrawDepth->Draw(m_depthRGBX, cDepthWidth * cDepthHeight * cBytesPerPixel);
			}

			// We're done with the texture so unlock it
			pTexture->UnlockRect(0);

			// Release the frame
			m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);
		}
		POINT3 SkeletonToScreen(POINT3 point, int width, int height) {
			//LONG x, y;
			//USHORT depth;
			POINT3 s;
			Vector4 v;
			v.x = point.x;
			v.y = point.y;
			v.z = point.z;
			v.w = point.w;
			float x_max = 4.4f / zoom;	// meters
			float y_max = 3.2f / zoom;	// meters

										/*NuiTransformSkeletonToDepthImage(v, &x, &y, &depth);
										float screenPointX = static_cast<float>(x);
										float screenPointY = static_cast<float>(y);
										float screenPointZ = static_cast<float>(depth);*/

			s.x = (float)(((float)width)*(((x_max / 2.0f) + point.x) / x_max)) + x_offset;
			s.y = (float)((((float)height)*((1 - ((y_max / 2.0f) + point.y)) / y_max))) + y_offset;
			s.z = point.z;
			s.w = 0.0f;
			return s;
		}
	};

}

//#endif#pragma once
