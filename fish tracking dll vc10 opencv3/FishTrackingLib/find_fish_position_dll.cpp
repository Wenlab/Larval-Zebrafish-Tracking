// find_fish_position_dll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "find_fish_position_dll.h"

#include "opencv2/opencv.hpp"
#include "opencv2/core/ocl.hpp"
#include <math.h>

#include <fstream>

//#define USEOCL

using namespace cv;
using namespace std;
//#include <NIVision.h>


// This is an example of an exported function.
FIND_FISH_POSITION_DLL_API int find_centroid(char* LVImagePtrSrc, int LVLineWidthSrc,
			      char* LVImagePtrDst, int LVLineWidthDst,
			      char* LVImagePtrBkg, int LVLineWidthBkg,
				  int LVWidth, int LVHeight,
				  int BkgWidth, int BkgHeight,
				  Point *position, double scale_x, double scale_y, double theta,
				  int BinThresh, int Esize1, int Esize2, int counting_thresh,
				  bool setROI, int width, int height, 
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

//to be deleted
	Point stage_position;
	stage_position = *position;
	stage_position.x = -stage_position.x;



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

	setROI = TRUE;

	Mat Bkg;
	Mat Bkg_global = cvarrToMat(CVImageBkg);
	//transform_coordination(position, BkgWidth, BkgHeight, scale_x, scale_y, theta);
	//crop_background(Bkg_global, Bkg, position,
	//	LVWidth, LVHeight);
//to be deleted
	transform_coordination(&stage_position, BkgWidth, BkgHeight, scale_x, scale_y, theta);
	crop_background(Bkg_global, Bkg, &stage_position,
		LVWidth, LVHeight);

	Rect Mask;

	Mat Src = cvarrToMat(CVImageSrc);
	Mat Dst = cvarrToMat(CVImageDst);


	//QueryPerformanceCounter(&timeEnd1);
	//double elapsed1 = (timeEnd1.QuadPart - timeStart.QuadPart) / quadpart;

	subtract(Src,Bkg,Dst);
	threshold(Dst,Dst,BinThresh,255,THRESH_BINARY);

	//QueryPerformanceCounter(&timeEnd2);
	//double elapsed2 = (timeEnd2.QuadPart - timeStart.QuadPart) / quadpart;

	//Mat element1=getStructuringElement(MORPH_RECT, Size(2*Esize1+1,2*Esize1+1),Point(Esize1,Esize1));
	Mat element1 = getStructuringElement(MORPH_RECT, Size(2 * Esize1 + 1, 2 * Esize1 + 1));

	//dilate(Dst, Dst,element1,Point(-1,-1),2);
	//erode(Dst, Dst,element1,Point(-1,-1),2);
	erode(Dst, Dst, element1);
	dilate(Dst, Dst, element1);

	//Mat element2=getStructuringElement(MORPH_RECT, Size(2*Esize2+1,2*Esize2+1),Point(Esize2,Esize1));
	Mat element2 = getStructuringElement(MORPH_RECT, Size(2 * Esize2 + 1, 2 * Esize2 + 1));

	erode(Dst, Dst,element2);
	dilate(Dst, Dst, element2);



	//QueryPerformanceCounter(&timeEnd3);
	//double elapsed3 = (timeEnd3.QuadPart - timeStart.QuadPart) / quadpart;





	/** Find Contours and use contours to find fish center**/



	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;


	findContours(Dst, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);





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


	tx = ((centroid.x - width / 2) > 0) ? centroid.x - width / 2 : 0;
	ty = ((centroid.y - height / 2) > 0) ? centroid.y - height / 2 : 0;

	ROI_width = ((centroid.x + width / 2) < LVWidth) ? width : LVWidth - centroid.x - 1 + width / 2;
	ROI_height = ((centroid.y + height / 2) < LVHeight) ? height : LVHeight - centroid.y - 1 + height / 2;

	if ((tx + ROI_width > LVWidth) || (ty + ROI_height > LVHeight)){

		tx = 0;
		ty = 0;
		ROI = Rect(0, 0, LVWidth, LVHeight);
	}


	else{


		ROI = Rect(tx, ty, ROI_width, ROI_height);

	}

	
	Mat Dst_cut(Dst, ROI);

	Mat DistImage(Dst_cut.size(), CV_32FC1);

	distanceTransform(Dst_cut, DistImage, CV_DIST_L1, 3);


	//QueryPerformanceCounter(&timeEnd5);
	//double elapsed5 = (timeEnd5.QuadPart - timeStart.QuadPart) / quadpart;


	double min,max;
	Point maxLoc;
	
	minMaxLoc(DistImage,&min,&max,NULL,&maxLoc);
		

	//if (shift == 0){

	//	centroid_out->x = maxLoc.x + tx;
	//	centroid_out->y = maxLoc.y + ty;

	//	if (max>0) {



	//		circle(Src,*centroid_out,width_fish/3,125);

	//		//QueryPerformanceCounter(&timeEnd);
	//		////得到两个时间的耗时  
	//		//double elapsed = (timeEnd.QuadPart - timeStart.QuadPart) / quadpart;
	//		//ofstream out;
	//		//out.open("E:\elapsed_time.txt", ios::out | ios::app);
	//		//out << "shift=0, FISH_FOUND, total: " << elapsed << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out << "prepare: " << elapsed1 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out << "subtract+blur+threshold: " << elapsed2 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out << "morphology: " << elapsed3 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out << "contour: " << elapsed4 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out << "distanceTransform: " << elapsed5 << endl;//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out.close();





	//		return FISH_FOUND;

	//	}

	//	else {





	//		//QueryPerformanceCounter(&timeEnd);
	//		////得到两个时间的耗时  
	//		//double elapsed = (timeEnd.QuadPart - timeStart.QuadPart) / quadpart;
	//		//ofstream out;
	//		//out.open("E:\elapsed_time.txt", ios::out | ios::app);
	//		//out << "shift=0, FISH_NOT_FOUND, total: " << elapsed << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out << "prepare: " << elapsed1 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out << "subtract+blur+threshold: " << elapsed2 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out << "morphology: " << elapsed3 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out << "contour: " << elapsed4 << "\t";//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out << "distanceTransform: " << elapsed5 << endl;//单位为秒，精度为微秒(1000000/cpu主频)
	//		//out.close();







	//		return FISH_NOT_FOUND;

	//	}


	//}


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

	float 	dtemp;
	float   d_t = 1;
	int     x_min = LVWidth;
	int     x_max = 0;
	int     y_min = LVHeight;
	int     y_max = 0;

	Point extreme_Pt1;
	Point extreme_Pt2;
	Point Midpoint;

	vector<Point>::iterator it;

	if (vx > 0.7071)//horizontal
	{
		for (it = rough.begin(); it != rough.end(); it++)
		{
			dtemp = GetdistFromCenterline(*it, centerline_Param); //calculate the distance between the boundary point and centerline
			if (dtemp < d_t){
				if (it->x < x_min)
				{
					x_min = it->x;
					extreme_Pt1 = *it;
				}
				if (it->x > x_max)
				{
					x_max = it->x;
					extreme_Pt2 = *it;
				}
			}
		}
	}
	else//vertical
	{
		for (it = rough.begin(); it != rough.end(); it++)
		{
			dtemp = GetdistFromCenterline(*it, centerline_Param); //calculate the distance between the boundary point and centerline
			if (dtemp < d_t){
				if (it->y < y_min)
				{
					y_min = it->y;
					extreme_Pt1 = *it;
				}
				if (it->y > y_max)
				{
					y_max = it->y;
					extreme_Pt2 = *it;
				}
			}
		}
	}


	/* determine the midpoint of the centerline  */

	Midpoint.x = (extreme_Pt1.x + extreme_Pt2.x)/2;
	Midpoint.y = (extreme_Pt1.y + extreme_Pt2.y)/2;


	/** for some unknown reason, labview output centroid x and y are swapped **/

	if (vx > 0.7071)//horizontal
	{
		if (Midpoint.x > maxLoc.x + tx) {

			centroid_out->y = maxLoc.y + shift*vy + ty;
			centroid_out->x = maxLoc.x + shift*vx + tx;

			heading_vector->x = -vx;
			heading_vector->y = -vy;

		}

		else{

			centroid_out->y = maxLoc.y - shift*vy + ty;
			centroid_out->x = maxLoc.x - shift*vx + tx;

			heading_vector->x = vx;
			heading_vector->y = vy;

		}
	}
	else//vertical
	{
		if ((Midpoint.y > maxLoc.y + ty) && vy>0 || (Midpoint.y < maxLoc.y + ty) && vy<0) {

			centroid_out->y = maxLoc.y + shift*vy + ty;
			centroid_out->x = maxLoc.x + shift*vx + tx;

			heading_vector->x = -vx;
			heading_vector->y = -vy;

		}

		else{

			centroid_out->y = maxLoc.y - shift*vy + ty;
			centroid_out->x = maxLoc.x - shift*vx + tx;

			heading_vector->x = vx;
			heading_vector->y = vy;

		}
	}




	//QueryPerformanceCounter(&timeEnd6);
	//double elapsed6 = (timeEnd6.QuadPart - timeStart.QuadPart) / quadpart;




	//calculate the Mask for updating the background excluding the fish

	//if (vx < 0.7071)
	//{
	//	Mask = Rect(Midpoint.x - 2*75 * vx - 2*15, Midpoint.y - 2*75 * abs(vy), 2*150 * vx + 2*30, 2*150 * abs(vy));
	//}
	//else
	//{
	//	Mask = Rect(Midpoint.x - 2*75 * vx, Midpoint.y - 2*75 * abs(vy) - 2*15, 2*150 * vx, 2*150 * abs(vy) + 2*30);
	//}
	if (vx < 0.7071)
	{
		Mask = Rect(Midpoint.x - (length_fish/2) * vx - (width_fish/2), Midpoint.y - (length_fish/2) * abs(vy), length_fish * vx + width_fish, length_fish * abs(vy));
	}
	else
	{
		Mask = Rect(Midpoint.x - (length_fish/2) * vx, Midpoint.y - (length_fish/2) * abs(vy) - (width_fish/2), length_fish * vx, length_fish * abs(vy) + width_fish);
	}

	Rect r2(0, 0, LVWidth, LVHeight);//the boundary of image

	Mask = Mask & r2;

	if (update_background(Bkg, Src, Mask, Update_Ratio_Bkg)!=0)
	{
		ofstream error_out;
		error_out.open("E:\error_out.txt", ios::out | ios::app);
		error_out << "update_background failed!" << endl;
		error_out.close();
	}

	rectangle(Src, Point(Mask.x, Mask.y), Point(Mask.x + Mask.width, Mask.y + Mask.height), 125);
	circle(Src,*centroid_out,width_fish/3,125);
	circle(Src,Midpoint,width_fish/2,255);





	//QueryPerformanceCounter(&timeEnd7);
	//double elapsed7 = (timeEnd7.QuadPart - timeStart.QuadPart) / quadpart;



	//QueryPerformanceCounter(&timeEnd);
	////得到两个时间的耗时  
	//double elapsed = (timeEnd.QuadPart - timeStart.QuadPart) / quadpart;
	//ofstream out;
	//out.open("E:\elapsed_time.txt", ios::out | ios::app);
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
		error_out.open("E:\error_out.txt", ios::out|ios::app);
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
	Bkg4 = Bkg(Rect(Mask.x+Mask.width,0,Bkg.cols-Mask.x-Mask.width,Mask.height));

	Src1 = Src(Rect(0,0,Bkg.cols,Mask.y));
	Src2 = Src(Rect(0,Mask.y+Mask.height,Bkg.cols,Bkg.rows-Mask.y-Mask.height));
	Src3 = Src(Rect(0,Mask.y,Mask.x,Mask.height));
	Src4 = Src(Rect(Mask.x+Mask.width,0,Bkg.cols-Mask.x-Mask.width,Mask.height));

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




