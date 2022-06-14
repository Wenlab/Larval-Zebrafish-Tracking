/***************************************************************************
*
* CirClassExample.cpp
*
* This application uses the CircularInterface class to acquire images
* into a circular buffer. This application is equivalent to the Circular.c
* application, the difference being the Circular.c application
* uses the Bi API and this application uses the CircularInterface class.
*
* There are several advantages to using the CircularInterface class
* that are demonstrated in this application.
* The advantages being:
*	1. Fewer method/function calls to accomplish sequence capture.
*	2. No need to call cleanup methods/functions.
*	3. Fewer method/function parameters.
*	4. Improved error handling by throwing exceptions.
*
* We encouraged you to compare this application to the Circular.c
* application to see the advantages for yourself.
*
* This example demonstrates the use of CircularInterface class.
*
* Copyright (C) 2004 by BitFlow, Inc.  All Rights Reserved.
*
***************************************************************************/



// note that: don't minmize the cmd window, ortherwise the process will be error!!!

#include "stdafx.h"
#include "CircClassExample.h"
#include "CircularInterface.h"
#include <conio.h>
#include "CiApi.h"
#include <math.h>
#include <windows.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
 
//tensorRT
#include"TRTruntime.h"

//opencv
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<opencv2/imgcodecs/imgcodecs.hpp>
#include<opencv2/imgproc.hpp>

//Consol Coordinate Read
#include "ConsolCoorRead.h"

//Voltage Input
#include "VoltageInput.h"

//MPC
#include "MPC_main.h"

//GUI
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_Double_Window.H>
#include <cstdlib>                   //for exit(0)
#include <string.h>
#include <sstream>
#include "trackingControl.h"

using namespace cv;
using namespace std;

#define DEBUG_FLAG 0

// TCP
#define TRACK_SERVER_PORT 11000
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#pragma comment(lib, "ws2_32.lib")

// Threads
UINT WaitForBufferDone(LPVOID lpdwParam);
UINT CirErrorThread(LPVOID lpdwParam);
UINT TRTImageProcessThread(LPVOID lpdwParam);
UINT FrameGUIThread(LPVOID lpdwParam);
UINT RecordCoorThread(LPVOID lpdwParam);

MSG		Msg;
BFBOOL	endTest = FALSE;
int		hDspSrf = -1;	// handle to display surface 
BFBOOL display = FALSE; // the default of display is TRUE

CWinApp theApp;

int TRTflag=0;

using namespace std;
using namespace BufferAcquisition;

struct bitflowTRTStru 
{
	BFU32 frameNum;
	void* imageDataBuffer;
	TRTruntime trt;
	bitflowTRTStru()
	{
	}
};
Mat test;

//tgd
//TRTruntime trt(1, IMG_SIZE, IMG_SIZE, 2);

BFU32 	rv;
BiCirHandle	cirHandle;
BFTickRec T0, T1;

//ConsolCoorRead
ConsolCoorRead consoleread;

//VoltageInput
VoltageInput voltage;
float64 voltage_x = 1.0;
float64 voltage_y = 1.0;

//Historical data
vector<Point2d> command_history;
vector<Point2d> position_history;
vector<Point2d> fish_tr_history_c;
vector<Point2d> fish_direction_history_c;

// parameters for MPC
//int command_history_length = 30;
//int predict_length = 6;
//int fish_history_length = 4;
//double gammaX = 0.03;
//double gammaY = 0.03;
//double max_command = 10.0;
//int max_shift_head2yolk = 29;
//double scale_x = 1.0 / 30.0;
//double scale_y = 1.0 / 30.0;
//double theta = atan(-0.043);
//double scale_x2 = 1.0 / 10000.0;
//double scale_y2 = 1.0 / 12800.0;
Point dst_fish_position = Point(160, 160);

//GUI
trackingParams* params;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(BFNULL), BFNULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		BFU32	boardType, boardNum, init, serNum;
		char	ch;
		BFU32	numBuffers = 50;
		BFU32	cirSetupOptions = 0;
		BFU32	errorMode = CirErStop;
		CWinThread* pErrorThread = NULL;
		CWinThread* pFrameDoneThread = NULL;
		CWinThread* pFrameIMGThread = NULL;
		CWinThread* pFrameGUI = NULL;
		CWinThread* pRecordCoorThread = NULL;

		//GUI
		params = new trackingParams;

		// track class
		bitflowTRTStru bt;

		if (DoBrdOpenDialog(BOD_BRD_NUM_NON_FAMILY | BOD_HIDEJUSTOPEN, FF_BITFLOW_MODERN, &boardType, &boardNum,
			&init, &serNum))
		{
			return -1;
		}



		try
		{
			//��ʼ��board
			cout << "Creating instance of circular interface." << endl;
			CircularInterface board(boardNum, numBuffers, errorMode,
				cirSetupOptions);


			// CHECK THE BIT DEPTH OF THE CAMERA
			cout << "bitType: " << board.getBrdInfo(BiCamInqBitsPerPix) << endl;

			//tensorRT
			cout << "load model....." << endl;
			TRTruntime trt(1, IMG_SIZE, IMG_SIZE, 2);
			trt.DeserializeModel("trackKeyPointModel_0607_unet_320crop.trt");
			trt.createInferenceContext();
			cout << "Deserialize TRT model Done." << endl;
			bt.trt = trt;
			bt.frameNum = -1;
			bt.imageDataBuffer = NULL;

			// Initialize the console coordinate reading task.
			consoleread.ConCoorInitialize();

			// Initialize the voltage input task.
			voltage.volInitialize();

			// clear the history
			command_history.clear();
			position_history.clear();
			fish_tr_history_c.clear();
			fish_direction_history_c.clear();

			// Initialize command_history with 0.
			for (int i = 0; i <= params->command_history_length; i++)
				command_history.push_back(Point2d(0, 0));

			// Create display surface to view sequence.	
			if (display)
			{
				if (!DispSurfCreate((PBFS32)&hDspSrf, board.getBrdInfo(BiCamInqXSize),
					board.getBrdInfo(BiCamInqYSize0),
					board.getBrdInfo(BiCamInqBitsPerPixDisplay), BFNULL))
				{
					cout << "Couldn't create display surface" << endl;
					return 1;
				}
			}




			pErrorThread = AfxBeginThread(CirErrorThread, &board, THREAD_PRIORITY_HIGHEST);
			if (pErrorThread == BFNULL)
				return 1;

			pFrameDoneThread = AfxBeginThread(WaitForBufferDone, &board, THREAD_PRIORITY_HIGHEST);
			if (pFrameDoneThread == BFNULL)
				return 1;

			//board.cirControl(BISTART, BiAsync);
			//board.cirControl(BIPAUSE, BiAsync);//pause and manually control the stage when start the program

			pFrameIMGThread = AfxBeginThread(TRTImageProcessThread, &bt, THREAD_PRIORITY_HIGHEST);
			if (pFrameIMGThread == BFNULL)
				return 1;

			pFrameGUI = AfxBeginThread(FrameGUIThread, &params, THREAD_PRIORITY_HIGHEST);
			if (pFrameGUI == BFNULL)
				return 1;

			pRecordCoorThread = AfxBeginThread(RecordCoorThread, &params, THREAD_PRIORITY_HIGHEST);
			if (pRecordCoorThread == BFNULL)
				return 1;
			

			//printf("\nPress G (as in Go) to start Acquisition ");
			//printf("	Press S to Stop Acquisition \n");
			//printf("Press P to Pause				Press C to Continue\n");
			//printf("Press A to Abort\n");
			//printf("Press X to exit test\n\n");


			//cout << "endTest: " << endTest << endl;
			board.cirControl(BISTART, BiAsync); // start the board at the first time
			while (!endTest)
			{
				//cout << "The main loop... " << endl;
				
				// Wait here for a keyboard stroke
				while (!params->flag_cb && !endTest)
				{
					if (PeekMessage(&Msg, BFNULL, 0, 0, PM_REMOVE))
						DispatchMessage(&Msg);
					
					if (TRTflag == 1 && bt.frameNum != cirHandle.FrameCount)
					{
						bt.frameNum = cirHandle.FrameCount;
						bt.imageDataBuffer = cirHandle.pBufData;
						if (cirHandle.FrameCount % 10000 == 0)
							cout << "current num:" << bt.frameNum << endl;
					}
					//cout << "Waiting for trigger..." << endl;
				}
				params->flag_cb = false;//as a trigger, set false immediately after the waiting loop
				cout << "trigger works!" << endl;
				if (!endTest)
					//ch = BFgetch();
					ch = params->action;
				else
				{
					ch = 'X';
					TRTflag = 2;
				}

				cout << endl;

				switch (toupper(ch))
				{
				//case 'G': // Start acquisition
				//	TRTflag = 1;
				//	board.cirControl(BISTART, BiAsync);
				//	cout << "Circular Acquisition Started." << endl;
				//	break;

				case 'P':// Pause acquisition
					TRTflag = 0;
					//board.cirControl(BIPAUSE, BiAsync);
					cout << "Circular Acquisition Paused." << endl;
					break;

				case 'C':// Resume acquisition
					TRTflag = 1;
					//board.cirControl(BIRESUME, BiAsync);
					cout << "Circular Acquisition Resumed." << endl;
					break;

				//case 'S':// Stop acquisition
				//	TRTflag = 2;
				//	board.cirControl(BISTOP, BiAsync);
				//	break;

				//case 'A':// Abort acquisition
				//	TRTflag = 2;
				//	board.cirControl(BIABORT, BiAsync);
				//	break;

				case 'X':// Exit application
					TRTflag = 2;
					if (board.getStartAcqFlag())
						board.cirControl(BIABORT, BiAsync);
					endTest = TRUE;
					break;

				default:
					cout << "Key not Recognized, Try Again" << endl;
					break;
				}

			}
			CString Str;
			Str.Format("\nCaptured %d Frames\nMissed %d Frames\n",
				board.getNumFramesCaptured(), board.getNumFramesMissed());
			cout << Str << endl;

			BFRC error = board.getCirError();
			while (error != BI_OK)
			{
				board.showError(error);
				error = board.getCirError();
			}

			cout << "\nPress Any Key to Continue." << endl;
			while (!BFkbhit())
			{
				/* needed for display window */
				if (PeekMessage(&Msg, BFNULL, 0, 0, PM_REMOVE))
					DispatchMessage(&Msg);
				else
					Sleep(0);
			}

			// absorb key stroke 
			if (BFkbhit()) BFgetch();

			// Close Display window
			if (display)
				DispSurfClose(hDspSrf);

			// No need to clean up resources, the destructor of the
			// CircularInterface class will handle clean up.

			// clean up
			consoleread.ConsolCoorStop();
			consoleread.ConCoorEnd();
			voltage.volInput(0, 0);
			voltage.volEnd();
		}
		catch (BFException e)
		{
			e.showErrorMsg();
			nRetCode = 1;
		}

		if (display)
		{
			if (DispSurfIsOpen(hDspSrf))
			{
				// Close Display window
				DispSurfClose(hDspSrf);
			}
		}

		// Wait for threads to end
		DWORD exitCode;
		while (GetExitCodeThread(pErrorThread->m_hThread, &exitCode) &&
			exitCode == STILL_ACTIVE)
		{
			Sleep(10);
		}

		while (GetExitCodeThread(pFrameDoneThread->m_hThread, &exitCode) &&
			exitCode == STILL_ACTIVE)
		{
			Sleep(10);
		}

		while (GetExitCodeThread(pFrameIMGThread->m_hThread, &exitCode) &&
			exitCode == STILL_ACTIVE)
		{
			Sleep(10);
		}

		while (GetExitCodeThread(pFrameGUI->m_hThread, &exitCode) &&
			exitCode == STILL_ACTIVE)
		{
			Sleep(10);
		}

		while (GetExitCodeThread(pRecordCoorThread->m_hThread, &exitCode) &&
			exitCode == STILL_ACTIVE)
		{
			Sleep(10);
		}

	}

	return nRetCode;
}


UINT WaitForBufferDone(LPVOID lpdwParam)
{
	
	CircularInterface* board = (CircularInterface*)lpdwParam;
	try
	{
		// loop until cleanup is called
		rv = board->waitDoneFrame(INFINITE, &cirHandle);
		

		if (display)
			BFTick(&T0);

		while (rv != BI_CANCEL_CIR_FRAME_DONE)
		{
			// print buffer info if a valid buffer has been acquired
			if (rv == BI_CIR_STOPPED)
				cout << "Circular Acquisition Stopped." << endl;
			else if (rv == BI_CIR_ABORTED)
				cout << "Circular Acquisition Aborted." << endl;
			else if (rv == BI_ERROR_CIR_WAIT_TIMEOUT)
				cout << "BiSeqWaitDone has timed out." << endl;
			else if (rv == BI_ERROR_CIR_WAIT_FAILED)
				cout << "The wait in BiSeqWaitDone Failed." << endl;
			else if (rv == BI_ERROR_QEMPTY)
				cout << "The queue was empty." << endl;
			else
			{
				if (display)
				{
					// only update display every 30 msec
					if (BFTickDelta(&T0, BFTick(&T1)) > 50)
					{
						// This new function will format the image data for display,
						// then display the image. This reduces a lot of code. (See
						// Circular.c)
						if (!DispSurfFormatBlit(hDspSrf, cirHandle.pBufData,    //image data pointer
							board->getBrdInfo(BiCamInqBitsPerPix), BFDISP_FORMAT_NORMAL))
						{
							cout << "Could not update the display surface" << endl;
							return 1;
						}
						BFTick(&T0);

					}
				}

				/*
				BFU32 pnextnum = -1;
				PBFQNode currentNote = cirHandle.pNode;
				BFU32 pnum = currentNote->BufferNumber;
				cout << "start num: " << pnum << endl;
				cout << endl;
				int a = 0;
				while (a!=5)
				{
					BFU32 currentnum = currentNote->BufferNumber;
					cout << "current BufferNumber: " << currentnum << endl;
					cout << "status: " << currentNote->Status << endl;
					currentNote = currentNote->Next;
					pnextnum = currentNote->BufferNumber;
					cout << "next BufferNumber: " << pnextnum << endl;
					cout << endl;
					a++;
				}
				cout << "-----------------------------------" << endl;

				*/
				
				/*
				if (board->getNumFramesCaptured()%1000==0)
				{
					cout << "caputured image num: " << board->getNumFramesCaptured() << endl;
				}
				*/

				// single thread:

				/*
				//TOdO: add track code
				vector<cv::Point> outputVec;
				
				void* image = cirHandle.pBufData;

				trt.launchInference(image, outputVec);

				cout << "frame num:" << cirHandle.FrameCount << endl;
				*/


				// Mark the buffer as AVAILABLE after processing
				board->setBufferStatus(cirHandle, BIAVAILABLE);

				// output some stats
				//cout << setfill('0');
				
				//cout << "Buffer: " << setw(8) << cirHandle.BufferNumber << " ";
				//cout << "captured num: " << board->getNumFramesCaptured() << endl;
			}

			rv = board->waitDoneFrame(INFINITE, &cirHandle);
			//cout << "grab thread" << endl;
		}
	}
	catch (BFException e)
	{
		e.showErrorMsg();

		// End application if an error occurs
		endTest = TRUE;

		return 1;
	}

	return 0;
}

UINT CirErrorThread(LPVOID lpdwParam)
{
	CircularInterface* board = (CircularInterface*)lpdwParam;

	while (board->cirErrorWait() != BI_CIR_CLEANUP)
	{
		cout << "ErrorThread - Acquisition Error!!" << endl;
	}

	return 0;
}

UINT FrameGUIThread(LPVOID lpdwParam)
{
	//trackingParams* params = (trackingParams*)lpdwParam;
	make_window(params);
	cout << "Start Fl::run()... " << endl;
	Fl::run();

	return 0;
}

UINT RecordCoorThread(LPVOID lpdwParam)
{
	Mat test4;

	//�����׽���
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Failed to load Winsock");
		return 1;
	}

	//�������ڼ������׽���
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(TRACK_SERVER_PORT); //1024���ϵĶ˿ں�
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (::bind(sockSrv, (LPSOCKADDR)&addrSrv, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {//Ϊ��������std::bind�����˸�::��
		printf("Failed bind:%d\n", WSAGetLastError());
		return 1;
	}

	if (listen(sockSrv, 10) == SOCKET_ERROR) {
		printf("Listen failed:%d", WSAGetLastError());
		return 1;
	}

	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	//�ȴ��ͻ�������    
	SOCKET sockConn = accept(sockSrv, (SOCKADDR *)&addrClient, &len);
	if (sockConn == SOCKET_ERROR) {
		printf("Accept failed:%d", WSAGetLastError());
		//break;
	}

	printf("Accept client IP:[%s]\n", inet_ntoa(addrClient.sin_addr));

	bool flag_client_closed = false;
	char recvBuf[100];
	int i;
	ofstream output;
	time_t nowtime;
	nowtime = time(NULL);
	struct tm *local;
	local = localtime(&nowtime);
	char char_time[100];
	char path_image[100];
	sprintf(char_time, "H:\\stage_position\\Stage_postion%d_%d_%d-%d_%d_%d.txt", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
	sprintf(path_image, "H:\\track_in_c\\%d_%d_%d-%d_%d_%d\\", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
	cout << "file path: " << char_time << endl;
	output.open(char_time, ios::out | ios::app);
	cout << "File(stage position) openning succeeds!\n";
	mkdir(path_image);
	cout << "Mkdir for image recording succeeds!\n";
	//�������ݣ�֡�ţ����洢��դ�߶���
	while (!endTest && !flag_client_closed)
	{
		//memset(recvBuf, 0, sizeof(recvBuf));
		if (recv(sockConn, recvBuf, sizeof(recvBuf), 0)<=0)//receive the trigger
		{
			flag_client_closed = true;//whether the client socket is closed
		}
		//cout << "recvBuf: " << recvBuf << endl;
		i = atoi(recvBuf);
		output << i << ", " << "x: " << consoleread.coordata[0] << ", " << "y: " << -consoleread.coordata[1];//record the frame number and coordinates
		output << ", detection: " << params->fish_detection << ", head: " << params->head << ", yolk: " << params->yolk;
		output << ", confidence_h: " << params->confidence_h << ", confidence_y: " << params->confidence_y << endl;

		//record the images
		if (i % 1 == 0)
		{
			test4 = test.clone();
			//cv::circle(test4, params->head, 6, 128, 2);
			//cv::circle(test4, params->yolk, 3, 128, 2);
			imwrite(path_image + cv::format("%.6d", i) + ".jpg", test4);
		}

		//cout << i << " " << consoleread.coordata[0] << " " << -consoleread.coordata[1] << endl;
	}
	output.close();

	closesocket(sockConn);
	closesocket(sockSrv);
	WSACleanup();

	return 0;
}

UINT TRTImageProcessThread(LPVOID lpdwParam)
{
	bitflowTRTStru* bt = (bitflowTRTStru*)lpdwParam;
	//CircularInterface* board = (CircularInterface*)lpdwParam;

	cout << "start TRT model process..." << endl;
	BFU32 frameCount = -2;
	BFU32 frameCount_processed = 0;

	
	test = Mat(cv::Size(IMG_SIZE, IMG_SIZE), CV_8UC1);
	Mat test2(cv::Size(IMG_SIZE, IMG_SIZE), CV_32FC1);
	Mat test3;

	try
	{
		while (1)
		{
			 
			//if (bt->frameNum % 1000 == 0 && bt->frameNum!=0)
			if(bt->frameNum==1)
				cout << "frameCount: " << frameCount << endl;

			 //cout << TRTflag << endl;

			if (TRTflag == 0)// manually control the stage
			{
				consoleread.ConCoorRead();
				//if (params->voltage_x != 0)
				//{
				//	cout << "volInput: " << params->voltage_x << ", " << params->voltage_y << endl;
				//}
				voltage.volInput(params->voltage_x, params->voltage_y);

				// clear the history
				command_history.clear();
				position_history.clear();
				fish_tr_history_c.clear();
				fish_direction_history_c.clear();

				// Initialize command_history with 0.
				for (int i = 0; i <= params->command_history_length; i++)
					command_history.push_back(Point2d(0, 0));

				continue;
			}
			else if (TRTflag  == 1)
			{
				//
				// cout << "1:" << bt->frameNum << endl;
				//cout << "Tracking branch begins." << endl;
				if (frameCount != bt->frameNum && bt->imageDataBuffer != NULL)
				{
					// elapsed time
					LARGE_INTEGER timeStart;    //��ʼʱ��  
					LARGE_INTEGER timeEnd,timeEnd1,timeEnd2,timeEnd3,timeEnd4,timeEnd5,timeEnd6,timeEnd7;      //����ʱ��  
					LARGE_INTEGER frequency;    //��ʱ��Ƶ��  
					QueryPerformanceFrequency(&frequency);  
					double quadpart = (double)frequency.QuadPart;//��ʱ��Ƶ��  
					QueryPerformanceCounter(&timeStart);  

					// read the coordinates of the current console location.
					//cout << "read a pair of console coordinates ..." << endl;
					consoleread.ConCoorRead();
					//printf("\n channel %u Current UpDown count: %u\n", 0, consoleread.coordata[0]);
					//printf("\n channel %u Current UpDown count: %u\n", 1, consoleread.coordata[1]);
					if (position_history.size() != params->fish_history_length)// If the size of position_history is not equal to fish_history_length, initialize it with the data of the present frame.
					{
						position_history.clear();
						for (int i = 0; i <= params->fish_history_length; i++)
							position_history.push_back(Point2d(consoleread.coordata[0], -consoleread.coordata[1]));// Note the sign!!!
					}
					else// Push back the present data, and pop the data at the beginning
					{
						position_history.push_back(Point2d(consoleread.coordata[0], -consoleread.coordata[1]));// Note the sign!!!
						vector<Point2d>::iterator it = position_history.begin();
						position_history.erase(it);
					}

					QueryPerformanceCounter(&timeEnd1);
					//cout << frameCount << "    " << bt->frameNum << endl;
					test.data = (uchar*) bt->imageDataBuffer;
					vector<cv::Point> outputVec;
					vector<float> confidence;
					
					test.convertTo(test2, CV_32FC1);
					test2 = test2 / 255;
					QueryPerformanceCounter(&timeEnd2);
					bt->trt.launchInference(test2, outputVec, confidence);
					QueryPerformanceCounter(&timeEnd3);

					params->head = outputVec[0];
					params->yolk = outputVec[1];
					params->confidence_h = confidence[0];
					params->confidence_y = confidence[1];

					double elapsed_ConCoorRead = (timeEnd1.QuadPart - timeStart.QuadPart) / quadpart;
					double elapsed_convertion = (timeEnd2.QuadPart - timeEnd1.QuadPart) / quadpart;
					double elapsed_process = (timeEnd3.QuadPart - timeEnd2.QuadPart) / quadpart;

					//imwrite("D:/tgd/Track-TEST/testImg/"+cv::format("%.4d", frameCount) + ".jpg", test2);
					

					frameCount = bt->frameNum;
					//if (frameCount % 10 == 0)
					//	cout << "processed frame count: " << bt->frameNum <<endl;

					
					//save image to test img process
					if (DEBUG_FLAG)
					{
						test3 = test.clone();
						for (int i = 0; i < outputVec.size(); i++)
						{
							cv::circle(test3, outputVec[i], 3, 128, 2);
						}
						imwrite("H:/track_in_c/testImg/" + cv::format("%.6d", frameCount) + ".jpg", test3);
					}

					Point2d fish_direction = Point2d(outputVec[0].x - outputVec[1].x, outputVec[0].y - outputVec[1].y);
					double shift_head2yolk = sqrt(fish_direction.x*fish_direction.x + fish_direction.y*fish_direction.y);
					if (shift_head2yolk > params->max_shift_head2yolk || shift_head2yolk < 3 || 
						(outputVec[1].x == 0 && outputVec[1].y == 0 && outputVec[0].x == 0 && outputVec[0].y == 0)||
						confidence[0] < params->threshold_confidence_h || confidence[1] < params->threshold_confidence_y)//fish detection error!!!
					{
						params->fish_detection = false;
						cout << "fish detection error!" << endl;
						cout << "head: " << outputVec[0] << endl;
						cout << "yolk: " << outputVec[1] << endl << endl;
						//voltage.volInput(0, 0);//Stop the stage while fish detection error.
						voltage.volInput(params->voltage_x, params->voltage_y);//�Ҳ������ʱ�򣬸��ֶ�����
						continue;//Do nothing else while fish detection error.
					}
					else
					{
						params->fish_detection = true;
					}
					fish_direction = Point2d(fish_direction.x / shift_head2yolk, fish_direction.y / shift_head2yolk);//normalization

					if (fish_tr_history_c.size() != params->fish_history_length)// If the size of fish_tr_history_c is not equal to fish_history_length, initialize it with the data of the present frame.
					{
						fish_tr_history_c.clear();
						for (int i = 0; i <= params->fish_history_length; i++)
							fish_tr_history_c.push_back(Point2d(outputVec[0].x, outputVec[0].y));
					}
					else// Push back the present data, and pop the data at the beginning
					{
						fish_tr_history_c.push_back(Point2d(outputVec[0].x, outputVec[0].y));
						vector<Point2d>::iterator it = fish_tr_history_c.begin();
						fish_tr_history_c.erase(it);
					}

					if (fish_direction_history_c.size() != params->fish_history_length)// If the size of fish_direction_history_c is not equal to fish_history_length, initialize it with the data of the present frame.
					{
						fish_direction_history_c.clear();
						for (int i = 0; i <= params->fish_history_length; i++)
							fish_direction_history_c.push_back(fish_direction);
					}
					else// Push back the present data, and pop the data at the beginning
					{
						fish_direction_history_c.push_back(fish_direction);
						vector<Point2d>::iterator it = fish_direction_history_c.begin();
						fish_direction_history_c.erase(it);
					}

					QueryPerformanceCounter(&timeEnd4);
					double elapsed_history = (timeEnd4.QuadPart - timeEnd3.QuadPart) / quadpart;

					//MPC
					dst_fish_position.x = params->dst_fish_position_x;
					dst_fish_position.y = params->dst_fish_position_y;
					Point2d c0 = MPC_main(params->command_history_length, params->predict_length, params->fish_history_length,
						params->gammaX, params->gammaY, params->max_command, shift_head2yolk,
						1.0/params->scale_x, 1.0/params->scale_y, atan(params->theta), 1.0/(params->scale_x2), 1.0/(params->scale_y2), &dst_fish_position,
						command_history, position_history, fish_tr_history_c, fish_direction_history_c);
					voltage_x = c0.x;
					voltage_y = -c0.y;// Note the sign!!!

					QueryPerformanceCounter(&timeEnd5);
					double elapsed_MPC = (timeEnd5.QuadPart - timeEnd4.QuadPart) / quadpart;

					// Output the voltage
					//voltage.volInput(voltage_x, voltage_y);
					if (params->voltage_x != 0 || params->voltage_y != 0)//������ֶ��������룬���Ȱ����ֶ�����
					{
						voltage.volInput(params->voltage_x, params->voltage_y);
					}
					else
					{
						voltage.volInput(voltage_x, voltage_y);
					}

					if (command_history.size() != params->command_history_length)// If the size of command_history is not equal to command_history_length, initialize it with 0.
					{
						command_history.clear();
						for (int i = 0; i <= params->command_history_length; i++)
							command_history.push_back(Point2d(0, 0));
					}
					else// Push back the present data, and pop the data at the beginning
					{
						command_history.push_back(Point2d(voltage_x, voltage_y));
						vector<Point2d>::iterator it = command_history.begin();
						command_history.erase(it);
					}

					// elapsed time
					QueryPerformanceCounter(&timeEnd);
					double elapsed = (timeEnd.QuadPart - timeStart.QuadPart) / quadpart;
					double elapsed_vector = (timeEnd.QuadPart - timeEnd5.QuadPart) / quadpart;

					// frameCount_processed
					frameCount_processed++;

					// output
					if (DEBUG_FLAG)
					{
						cout << "console coordinate: " << consoleread.coordata[0] << ", " << -consoleread.coordata[1] << endl;// Note the sign!!!
						cout << "head: " << outputVec[0] << endl;
						cout << "yolk: " << outputVec[1] << endl;
						cout << "voltage output: " << voltage_x << "," << voltage_y << endl;
						cout << "ConCoorRead elapsed time: " << elapsed_ConCoorRead << endl;
						cout << "Image convertion elapsed time: " << elapsed_convertion << endl;
						cout << "Image process elapsed time: " << elapsed_process << endl;
						cout << "History preparation elapsed time: " << elapsed_history << endl;
						cout << "MPC elapsed time: " << elapsed_MPC << endl;
						cout << "vector operation elapsed time: " << elapsed_vector << endl;
						cout << "elapsed time: " << elapsed << endl;
						cout << "frameCount processed: " << frameCount_processed << endl << endl;
					}
				}
			}
			else if (TRTflag == 2)
			{
				cout << "frameCount processed: " << frameCount_processed << endl << endl;
				cout << "break" << endl;
				exit(0);//��ò�Ҫ���������ǲ�֪��Fl::run()��ôͣ����ʱ�������˳�����
				break;
			}
		}
	}
	catch (BFException e)
	{
		e.showErrorMsg();

		// End application if an error occurs
		endTest = TRUE;

		return 1;
	}

	return 0;
}
