// find_fish_position_dll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "find_fish_position_dll.h"

#include "opencv2/opencv.hpp"
#include "opencv2/core/ocl.hpp"
#include <math.h>

#include <fstream>

//#define USEOCL

#define PI 3.1415927

using namespace cv;
using namespace std;
//#include <NIVision.h>


// This is an example of an exported function.
FIND_FISH_POSITION_DLL_API int find_centroid(
		char* LVImagePtrSrc, int LVLineWidthSrc,
		char* LVImagePtrDst, int LVLineWidthDst,
		char* LVImagePtrBkg, int LVLineWidthBkg,
		int LVWidth, int LVHeight,
		int BkgWidth, int BkgHeight,
		unsigned char* templates, int n_templates, int TemplatesSize,
		Point *position, double scale_x, double scale_y, double theta,
		int BinThresh, int counting_thresh,
		int width, int height, 
		int width_fish, int length_fish, 
		int shift,Point *centroid_in, Point *centroid_out, Point2d *heading_vector,
		double Update_Ratio_Bkg)

{
	//LARGE_INTEGER timeStart;    //开始时间  
 //   LARGE_INTEGER timeEnd,timeEnd1,timeEnd2,timeEnd3,timeEnd4,timeEnd5,timeEnd6,timeEnd7;      //结束时间  
 //   LARGE_INTEGER frequency;    //计时器频率  
 //   QueryPerformanceFrequency(&frequency);  
 //   double quadpart = (double)frequency.QuadPart;//计时器频率  
 // 
 //   QueryPerformanceCounter(&timeStart);  

	Point stage_position;
	stage_position = *position;



	#ifndef USEOCL
    ocl::setUseOpenCL(!ocl::useOpenCL());
	#endif // USEOCL
    //cout << "Switched to " << (ocl::useOpenCL() ? "OpenCL enabled" : "CPU") << " mode\n";
	
		
	IplImage *CVImageSrc, *CVImageDst, *CVImageBkg;
	
	CvSize ImageSize,BkgGlobalSize;


	int tx, ty, ROI_width, ROI_height;

	tx=0;
	ty=0;


	ROI_width=LVWidth;
	ROI_height=LVHeight;


	ImageSize = cvSize(LVWidth,LVHeight);
	BkgGlobalSize = cvSize(BkgWidth, BkgHeight);
	
	CVImageSrc = cvCreateImageHeader(ImageSize, IPL_DEPTH_8U, 1);
	CVImageSrc->imageData = LVImagePtrSrc;
	CVImageSrc->widthStep = LVLineWidthSrc;
	CVImageDst = cvCreateImageHeader(ImageSize, IPL_DEPTH_8U, 1);
	CVImageDst->imageData = LVImagePtrDst;
	CVImageDst->widthStep = LVLineWidthDst;
	CVImageBkg = cvCreateImageHeader(BkgGlobalSize, IPL_DEPTH_8U, 1);
	CVImageBkg->imageData = LVImagePtrBkg;
	CVImageBkg->widthStep = LVLineWidthBkg;

	bool setROI = TRUE;

	Mat Bkg;
	Mat Bkg_global = cvarrToMat(CVImageBkg);
	//transform_coordination(position, BkgWidth, BkgHeight, scale_x, scale_y, theta);
	//crop_background(Bkg_global, Bkg, position,
	//	LVWidth, LVHeight);

	transform_coordination(&stage_position, BkgWidth, BkgHeight, scale_x, scale_y, theta);
	crop_background(Bkg_global, Bkg, &stage_position,
		LVWidth, LVHeight);

	Rect Mask;

	Mat Src = cvarrToMat(CVImageSrc);
	Mat Dst = cvarrToMat(CVImageDst);
	Mat Binary, Temp;

	vector<Mat> Templates;
	Templates.clear();
	for (int i = 0; i < n_templates; i++)
	{
		Temp = Mat(TemplatesSize, TemplatesSize, CV_8UC1, templates + sizeof(unsigned char)*TemplatesSize*TemplatesSize*i);
		Templates.push_back(Temp);
	}


	//QueryPerformanceCounter(&timeEnd1);
	//double elapsed1 = (timeEnd1.QuadPart - timeStart.QuadPart) / quadpart;

	subtract(Src,Bkg, Dst);
	threshold(Dst, Binary, BinThresh, 255, THRESH_BINARY);
	erode(Binary, Binary, Mat());
	dilate(Binary, Binary, Mat());

	//QueryPerformanceCounter(&timeEnd2);
	//double elapsed2 = (timeEnd2.QuadPart - timeStart.QuadPart) / quadpart;


	//QueryPerformanceCounter(&timeEnd3);
	//double elapsed3 = (timeEnd3.QuadPart - timeStart.QuadPart) / quadpart;





	/** Find Contours and use contours to find fish center**/



	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;


	findContours(Binary, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);





	//QueryPerformanceCounter(&timeEnd4);
	//double elapsed4 = (timeEnd4.QuadPart - timeStart.QuadPart) / quadpart;





	int matching_index = 0;

	Point refPt = Point(centroid_in->x, centroid_in->y);
	Point centroid = Point(0, 0);

	/** Find the Contour that is closed to the reference point **/
	if (contours.size()){

		/** if the contour area is too small, return no fish was found **/

		if (TargetContour(contours, refPt, counting_thresh, &matching_index, &centroid) != 0){
			centroid_out->x = centroid_in->x;
			centroid_out->y = centroid_in->y;




			//QueryPerformanceCounter(&timeEnd);
			////得到两个时间的耗时  
			//double elapsed = (timeEnd.QuadPart - timeStart.QuadPart) / quadpart;
			//ofstream out;
			//out.open("E:\elapsed_time.txt", ios::out | ios::app);
			//out << "no large contour, total: " << elapsed << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
			//out << "prepare: " << elapsed1 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
			//out << "subtract+blur+threshold: " << elapsed2 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
			//out << "morphology: " << elapsed3 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
			//out << "contours: " << elapsed4 << std::endl;//单位为秒，精度为微秒(1000000/cpu主频)
			//out.close();



			return FISH_NOT_FOUND;
		}
	}

	else{

		centroid_out->x = centroid_in->x;
		centroid_out->y = centroid_in->y;







		//QueryPerformanceCounter(&timeEnd);
		////得到两个时间的耗时  
		//double elapsed = (timeEnd.QuadPart - timeStart.QuadPart) / quadpart;
		//ofstream out;
		//out.open("E:\elapsed_time.txt", ios::out | ios::app);
		//out << "no contour, total: " << elapsed << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
		//out << "prepare: " << elapsed1 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
		//out << "subtract+blur+threshold: " << elapsed2 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
		//out << "morphology: " << elapsed3 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
		//out << "contours: " << elapsed4 << std::endl;//单位为秒，精度为微秒(1000000/cpu主频)
		//out.close();






		return FISH_NOT_FOUND;
	}

	
	Rect ROI;


	// width, height: the width or height of ROI
	// LVWidth, LVHeight: the width or height of field of view
	tx = ((centroid.x - width / 2) > 0) ? centroid.x - width / 2 : 0;
	ty = ((centroid.y - height / 2) > 0) ? centroid.y - height / 2 : 0;

	ROI_width = ((centroid.x + width / 2) < LVWidth) ? width : LVWidth - centroid.x - 1 + width / 2;
	ROI_height = ((centroid.y + height / 2) < LVHeight) ? height : LVHeight - centroid.y - 1 + height / 2;

	if (ROI_width < TemplatesSize || ROI_height < TemplatesSize)	//ROI should be larger than templates
	{
		centroid_out->x = centroid_in->x;
		centroid_out->y = centroid_in->y;

		return FISH_NOT_FOUND;
	}

	if ((tx + ROI_width > LVWidth) || (ty + ROI_height > LVHeight)){

		tx = 0;
		ty = 0;
		ROI = Rect(0, 0, LVWidth, LVHeight);
	}


	else{


		ROI = Rect(tx, ty, ROI_width, ROI_height);

	}

	
	//QueryPerformanceCounter(&timeEnd5);
	//double elapsed5 = (timeEnd5.QuadPart - timeStart.QuadPart) / quadpart;


	Mat Src_cut(Src, ROI);

	vector<Point> rough;

	rough = contours[matching_index];


/** if shift is not zero, a small compensation needs to be calculated **/


/** Calculate the centerline, tail, nose, midpoint of the fish **/

	Vec4f centerline_Param;
	float vx,vy,x0,y0;

	fitLine(rough, centerline_Param, CV_DIST_L2,0,0.01,0.01);

	
	vx = centerline_Param[0];
	vy = centerline_Param[1];
	x0 = centerline_Param[2];
	y0 = centerline_Param[3];

	//QueryPerformanceCounter(&timeEnd6);
	//double elapsed6 = (timeEnd6.QuadPart - timeStart.QuadPart) / quadpart;


	double thetaHeading = atan2(-vy, vx);
	if (thetaHeading < 0)
	{
		thetaHeading += 2 * PI;
	}
	double delta_theta = 2 * PI / Templates.size();
	int index = round(thetaHeading / delta_theta);

	Mat Result;
	Mat Template;
	double minVal_matching, maxVal_matching;
	double minVal_matching_global = INT_MAX;
	double maxVal_matching_global = INT_MIN;
	int index_template;
	vector<Point> minLoc_matching, maxLoc_matching;
	int indices[4]; // fitline结果方向的附近，及反方向的附近
	indices[0] = index;
	indices[1] = (index+1) % Templates.size();
	indices[2] = (index+Templates.size()/2) % Templates.size(); // Templates.size() should be even
	indices[3] = (index+1+Templates.size()/2) % Templates.size();
	for (int i = 0; i < Templates.size(); i++)
	{
		bool in_range = (i == indices[0]) || (i == indices[1]) || (i == indices[2]) || (i == indices[3]);
		if (!in_range)
		{
			minLoc_matching.push_back(Point(0, 0));
			maxLoc_matching.push_back(Point(0, 0));
			continue;
		}
		//template matching
		Template = Templates[i];
		matchTemplate(Src_cut, Template, Result, CV_TM_CCOEFF_NORMED);
		Point minLoc_matching_temp, maxLoc_matching_temp;
		minMaxLoc(Result, &minVal_matching, &maxVal_matching, &minLoc_matching_temp, &maxLoc_matching_temp);
		minLoc_matching_temp.x += tx;
		minLoc_matching_temp.y += ty;
		maxLoc_matching_temp.x += tx;
		maxLoc_matching_temp.y += ty;
		minLoc_matching.push_back(minLoc_matching_temp);
		maxLoc_matching.push_back(maxLoc_matching_temp);
		if (minVal_matching < minVal_matching_global)
			minVal_matching_global = minVal_matching;
		if (maxVal_matching > maxVal_matching_global)
		{
			maxVal_matching_global = maxVal_matching;
			index_template = i;
		}
	}
	//normalize(Result, Result, 0, 1, NORM_MINMAX, -1, Mat());
	//namedWindow("Result", CV_WINDOW_AUTOSIZE);
	//imshow("Result", Result);
	//waitKey(0);
	//namedWindow("Template", CV_WINDOW_AUTOSIZE);
	//imshow("Template", Templates[index_template]);
	//Mat Location;
	//Src.copyTo(Location);
	double angle = index_template * 2.0 / Templates.size() * PI;
	vx = cos(angle);
	vy = -sin(angle); // y axis is downward
	heading_vector->x = vx;
	heading_vector->y = vy;
	centroid_out->x = maxLoc_matching[index_template].x + Templates[index_template].cols / 2 + vx * shift;
	centroid_out->y = maxLoc_matching[index_template].y + Templates[index_template].rows / 2 + vy * shift;
	//rectangle(Location, maxLoc_matching[index_template], Point(maxLoc_matching[index_template].x + Templates[index_template].cols, maxLoc_matching[index_template].y + Templates[index_template].rows), Scalar::all(255), 2, 8, 0);
	//rectangle(Location, ROI, 125);
	//line(Location, Point(centroid_out->x - 50 * vx, centroid_out->y - 50 * vy), Point(centroid_out->x + 50 * vx, centroid_out->y + 50 * vy), Scalar::all(0));
	//circle(Location, *centroid_out, 3, Scalar::all(0));
	//imshow("Location", Location);


	//calculate the Mask for updating the background excluding the fish

	//if (vx < 0.7071)
	//{
	//	Mask = Rect(Midpoint.x - 2*75 * vx - 2*15, Midpoint.y - 2*75 * abs(vy), 2*150 * vx + 2*30, 2*150 * abs(vy));
	//}
	//else
	//{
	//	Mask = Rect(Midpoint.x - 2*75 * vx, Midpoint.y - 2*75 * abs(vy) - 2*15, 2*150 * vx, 2*150 * abs(vy) + 2*30);
	//}
	//if (vx < 0.7071)
	//{
	//	Mask = Rect(Midpoint.x - (length_fish/2) * vx - (width_fish/2), Midpoint.y - (length_fish/2) * abs(vy), length_fish * vx + width_fish, length_fish * abs(vy));
	//}
	//else
	//{
	//	Mask = Rect(Midpoint.x - (length_fish/2) * vx, Midpoint.y - (length_fish/2) * abs(vy) - (width_fish/2), length_fish * vx, length_fish * abs(vy) + width_fish);
	//}
	Mask = Rect(maxLoc_matching[index_template], Point(maxLoc_matching[index_template].x + Templates[index_template].cols, maxLoc_matching[index_template].y + Templates[index_template].rows));

	Rect r2(0, 0, LVWidth, LVHeight);//the boundary of image

	Mask = Mask & r2;

	if (update_background(Bkg, Src, Mask, Update_Ratio_Bkg)!=0)
	{
		ofstream error_out;
		error_out.open("E:\\error_out.txt", ios::out | ios::app);
		error_out << "update_background failed!" << endl;
		error_out.close();
	}

	//rectangle(Src, Point(Mask.x, Mask.y), Point(Mask.x + Mask.width, Mask.y + Mask.height), 125);
	//circle(Src,*centroid_out,width_fish/2,255);
	//circle(Src,Midpoint,width_fish/2,255);
	rectangle(Dst, maxLoc_matching[index_template], Point(maxLoc_matching[index_template].x + Templates[index_template].cols, maxLoc_matching[index_template].y + Templates[index_template].rows), Scalar::all(255), 2, 8, 0);
	rectangle(Dst, ROI, 125);
	line(Dst, Point(centroid_out->x - 50 * vx, centroid_out->y - 50 * vy), Point(centroid_out->x + 50 * vx, centroid_out->y + 50 * vy), Scalar::all(0));
	circle(Dst, *centroid_out, 3, Scalar::all(0));
	//namedWindow("Result", CV_WINDOW_AUTOSIZE);
	//imshow("Result", Dst);
	//waitKey(1);





	//QueryPerformanceCounter(&timeEnd7);
	//double elapsed7 = (timeEnd7.QuadPart - timeStart.QuadPart) / quadpart;



	//QueryPerformanceCounter(&timeEnd);
	////得到两个时间的耗时  
	//double elapsed = (timeEnd.QuadPart - timeStart.QuadPart) / quadpart;
	//ofstream out;
	//out.open("E:\\elapsed_time.txt", ios::out | ios::app);
	//out << "total: " << elapsed << "s" << endl;
	//if (elapsed > 0.002) out << "timeout" << endl;
	//out << "shift=1, FISH_FOUND, total: " << elapsed << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//out << "prepare: " << elapsed1 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//out << "subtract+blur+threshold: " << elapsed2 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//out << "morphology: " << elapsed3 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//out << "contour: " << elapsed4 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//out << "distanceTransform: " << elapsed5 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//out << "calculate_shift: " << elapsed5 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//out << "update_background: " << elapsed7 << endl;//单位为秒，精度为微秒(1000000/cpu主频)
	//out.close();

	//if (elapsed>0.01)
	//{
	//		ofstream time_out;
	//		time_out.open("E:\\time_out.txt", ios::out|ios::app);
	//		time_out << "time_out" << endl;
	//		time_out.close();
	//}



	//ofstream error_out;
	//		error_out.open("E:\error_out.txt", ios::out|ios::app);
	//		error_out << "FISH_FOUND shift==1" << endl;
	//		error_out.close();



	return FISH_FOUND;

}






void transform_coordination(Point *position, int BkgWidth, int BkgHeight, double scale_x, double scale_y, double theta)
{
	//transform to global background coordination
	double x = -cos(theta)*position->x - sin(theta)*position->y;
	double y = sin(theta)*position->x - cos(theta)*position->y;
	position->x = scale_x*x + BkgWidth / 2;
	position->y = scale_y*y + BkgHeight / 2;
}

void crop_background(Mat &Bkg_global, Mat &Bkg, Point *position,
	int LVWidth, int LVHeight)
{
	if ( (position->x < LVWidth / 2) || (position->x > Bkg_global.cols - LVWidth / 2) || (position->y < LVHeight / 2) || (position->y > Bkg_global.rows - LVHeight / 2) )
	{
		ofstream error_out;
		error_out.open("E:\\error_out.txt", ios::out|ios::app);
		error_out << "background cropping overflow!" << endl;
		error_out.close();
	}

	//crop the background
	Bkg = Bkg_global(Rect(position->x - LVWidth / 2, position->y - LVHeight / 2, LVWidth, LVHeight));
}

int update_background(Mat &Bkg, Mat &Src, Rect Mask, double Update_Ratio_Bkg)
{
	int flag = -1;

	Mat Bkg1,Bkg2,Bkg3,Bkg4,Src1,Src2,Src3,Src4;
	Bkg1 = Bkg(Rect(0,0,Bkg.cols,Mask.y));
	Bkg2 = Bkg(Rect(0,Mask.y+Mask.height,Bkg.cols,Bkg.rows-Mask.y-Mask.height));
	Bkg3 = Bkg(Rect(0,Mask.y,Mask.x,Mask.height));
	Bkg4 = Bkg(Rect(Mask.x+Mask.width, Mask.y,Bkg.cols-Mask.x-Mask.width,Mask.height));

	Src1 = Src(Rect(0,0,Bkg.cols,Mask.y));
	Src2 = Src(Rect(0,Mask.y+Mask.height,Bkg.cols,Bkg.rows-Mask.y-Mask.height));
	Src3 = Src(Rect(0,Mask.y,Mask.x,Mask.height));
	Src4 = Src(Rect(Mask.x+Mask.width, Mask.y,Bkg.cols-Mask.x-Mask.width,Mask.height));

	addWeighted(Src1, Update_Ratio_Bkg, Bkg1, 1 - Update_Ratio_Bkg, 0.0, Bkg1);
	addWeighted(Src2, Update_Ratio_Bkg, Bkg2, 1 - Update_Ratio_Bkg, 0.0, Bkg2);
	addWeighted(Src3, Update_Ratio_Bkg, Bkg3, 1 - Update_Ratio_Bkg, 0.0, Bkg3);
	addWeighted(Src4, Update_Ratio_Bkg, Bkg4, 1 - Update_Ratio_Bkg, 0.0, Bkg4);

	flag = 0;
	return flag;
}

int TargetContour(vector<vector <Point>> contours, Point refPt, int minArea, int *matching_index, Point *targetCentroid){


	double Dist;
	double minDist = 10000;
	double a = 0;
	Moments mu;
	Point comPt;
	int targetfound = -1;

	for (size_t k = 0; k < contours.size(); k++){

		mu = moments(contours[k], false);
		comPt = Point(static_cast<int>(mu.m10 / mu.m00), static_cast<int>(mu.m01 / mu.m00));
		a = mu.m00;

		if (a >= minArea){

			Dist = norm(comPt - refPt);
			if (Dist<minDist)
			{
				minDist = Dist;
				*matching_index = k;
				targetCentroid->x = comPt.x;
				targetCentroid->y = comPt.y;
				targetfound = 0;
			}
			//printf("Current is the target!\n");

		}
	}

	return targetfound;
}

int Max(int a, int b){

	if (a>=b)
		
		return a;

	else

		return b;
	
}

float GetdistFromCenterline(Point Pt, Vec4f centerline_Param){

	float vx,vy,x0,y0,ux,uy,dot,D;
	int x1,y1;

	vx = centerline_Param[0];
	vy = centerline_Param[1];
	x0 = centerline_Param[2];
	y0 = centerline_Param[3];


	x1 = Pt.x - x0; 
	y1 = Pt.y - y0;
	D = sqrt((float)x1*x1+(float)y1*y1);
	ux = x1/D;
	uy = y1/D;
	dot = ux*vx+uy*vy;
	return D*sqrt(1-dot*dot);


}

double dot_product(Point2d v1, Point2d v2)
{
	return v1.x*v2.x + v1.y*v2.y;
}


