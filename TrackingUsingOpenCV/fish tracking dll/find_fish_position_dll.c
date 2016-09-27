//#include <utility.h>
#include <NIVision.h>


#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <time.h>
#include <math.h>
#include "find_fish_position_dll.h"


int find_centroid(char* LVImagePtrSrc, int LVLineWidthSrc,
			      char* LVImagePtrDst, int LVLineWidthDst,
				  int LVWidth, int LVHeight, bool setROI,
				  int BinThresh, int Esize1, int Esize2, 
				  int width, int height, int shift,
				  Point *centroid_in, Point *centroid_out)

{

	IplImage *CVImageSrc, *CVImageDst;
	
	CvSize ImageSize;

	//clock_t start_t, end_t;

	int tx, ty, ROI_width, ROI_height;

	tx=0;
	ty=0;


	ROI_width=LVWidth;
	ROI_height=LVHeight;


	//double total_t;

	//start_t = clock();

	ImageSize=cvSize(LVWidth,LVHeight);
	
	CVImageSrc = cvCreateImageHeader(ImageSize,IPL_DEPTH_8U,1);
	CVImageSrc->imageData = LVImagePtrSrc;
	CVImageSrc->widthStep = LVLineWidthSrc;
	CVImageDst = cvCreateImageHeader(ImageSize, IPL_DEPTH_8U, 1);
	CVImageDst->imageData = LVImagePtrDst;
	CVImageDst->widthStep = LVLineWidthDst;
	cvZero(CVImageDst);

	/** Use Distance transform to find fish center **/
	IplImage* TempImage=cvCreateImage(ImageSize,IPL_DEPTH_32F,1);
	IplImage* TempImage2=cvCreateImage(ImageSize,IPL_DEPTH_8U,1);
	cvZero(TempImage);
	cvZero(TempImage2);

	if ((centroid_in->y >= LVWidth) || (centroid_in->x >= LVHeight)){

		setROI = FALSE;
	}

		


	//for some unknown reason the input centroid_in->x and centroid_in->y were swapped 

	if (setROI) {

		
		CvRect ROI;


		tx = ((centroid_in->y-width/2) > 0) ? centroid_in->y-width/2 : 0;
		ty = ((centroid_in->x-height/2) > 0) ? centroid_in->x-height/2 : 0;

		ROI_width = ((centroid_in->y+width/2) < LVWidth) ? width : LVWidth-centroid_in->y-1+width/2;
		ROI_height = ((centroid_in->x+height/2) < LVHeight) ? height : LVHeight-centroid_in->x-1+height/2;

		if ((tx+ROI_width > LVWidth) || (ty+ROI_height > LVHeight)){

			tx = 0;
			ty = 0;
			ROI = cvRect (0,0,LVWidth,LVHeight);
		}


		else{

			ROI = cvRect(tx,ty,ROI_width,ROI_height);

		}


		cvSetImageROI(CVImageSrc,ROI);
		cvSetImageROI(CVImageDst,ROI);
		cvSetImageROI(TempImage,ROI);
		cvSetImageROI(TempImage2,ROI);

	}



	cvThreshold(CVImageSrc,CVImageDst,BinThresh,255,CV_THRESH_BINARY);
	

	IplConvKernel* element1=cvCreateStructuringElementEx(Esize1,Esize1,Esize1/2,Esize1/2,CV_SHAPE_RECT,NULL);

	if (Esize1<3 || Esize1>10) Esize1=3;


	cvDilate(CVImageDst, CVImageDst,element1,2);
	cvErode(CVImageDst, CVImageDst,element1,2);

	cvReleaseStructuringElement(&element1);


	IplConvKernel* element2=cvCreateStructuringElementEx(Esize2,Esize2,Esize2/2,Esize2/2,CV_SHAPE_RECT,NULL);

	if (Esize2<3 || Esize2>10) Esize2=3;

	cvErode(CVImageDst, CVImageDst,element2,1);
	//cvDilate(CVImageDst, CVImageDst,element2,1);

	cvReleaseStructuringElement(&element2);

	
	
	//if (setROI) {cvSetImageROI(TempImage,ROI);}
	
	cvDistTransform(CVImageDst,TempImage, CV_DIST_L1,3,NULL,NULL);

	double min,max;
	CvPoint* maxLoc = (CvPoint*) malloc(sizeof(CvPoint));
	
	cvMinMaxLoc(TempImage,&min,&max,NULL,maxLoc,NULL);
		
	
	cvReleaseImage(&TempImage);

	//end_t = clock(); 

	//total_t=((double) end_t-start_t) / CLOCKS_PER_SEC;

	//total_t = end_t-start_t;

	//return (int) total_t*1000;

	

	/** Find Contours and use contours to find fish center**/


	CvSeq* contours;
	
	
	cvCopy(CVImageDst,TempImage2);

	CvMemStorage* MemStorage=cvCreateMemStorage(0);

	cvFindContours(TempImage2,MemStorage, &contours,sizeof(CvContour),CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE,cvPoint(0,0));


	CvSeq* rough;
	/** Find Longest Contour **/
	if (contours){

		LongestContour(contours,&rough);
	}

/** for some unknown reason, labview output centroid x and y are swapped **/

		
	else{

	 	centroid_out->x=centroid_in->x+ty;
		centroid_out->y=centroid_in->y+tx;
		cvReleaseImage(&TempImage2);
		cvReleaseMemStorage(&MemStorage);
		return 0;
	}


/** for some unknown reason, labview output centroid x and y are swapped **/

	if (shift==0){

		centroid_out->y=maxLoc->x+tx;
		centroid_out->x=maxLoc->y+ty;

		if (setROI){

			cvResetImageROI(CVImageDst);
			cvResetImageROI(CVImageSrc);
		}
		
		cvReleaseImage(&TempImage2);
		cvReleaseMemStorage(&MemStorage);
		return 0;

	}


/** if shift is not zero, a small compensation needs to be calculated **/


/** Calculate the centerline, tail, nose, midpoint of the fish **/

	float centerline_Param[4];
	float vx,vy,x0,y0;

	cvFitLine(rough, CV_DIST_L2,0,0.01,0.01,centerline_Param);

	vx = centerline_Param[0];
	vy = centerline_Param[1];
	x0 = centerline_Param[2];
	y0 = centerline_Param[3];

	
	
	int 	TotalBpts;
	int 	i;
	float 	dtemp;
	float 	dmin1 = 3000;
	float 	dmin2 = 3000;
	int   	idx1, idx2;


	CvPoint* Pt = (CvPoint*) malloc(sizeof(CvPoint));
	CvPoint* extreme_Pt1 = (CvPoint*) malloc(sizeof(CvPoint));
	CvPoint* extreme_Pt2 = (CvPoint*) malloc(sizeof(CvPoint));
	CvPoint* Midpoint = (CvPoint*) malloc(sizeof(CvPoint));

	TotalBpts = rough->total;

	 /* find point on the contour that is closest to the centerline */
		
	for (i=0; i<TotalBpts; i++) {
			
		Pt = (CvPoint*)cvGetSeqElem(rough,i);
		dtemp = GetdistFromCenterline(*Pt, centerline_Param); //calculate the distance between the boundary point and centerline

		if (dtemp < dmin1){
			dmin1 = dtemp;
			idx1 = i;

		}
		
		
	}





	/* find the second point on the contour that is closest to the centerline */

	for (i=idx1+TotalBpts/4; i<TotalBpts; i++){

		Pt = (CvPoint*)cvGetSeqElem(rough,i);
		dtemp = GetdistFromCenterline(*Pt, centerline_Param); //calculate the distance between the boundary point and centerline

		if (dtemp < dmin2){
			dmin2 = dtemp;
			idx2 = i;

		}


	}

	for (i=0; i<idx1-TotalBpts/4; i++){

		Pt = (CvPoint*)cvGetSeqElem(rough,i);
		dtemp = GetdistFromCenterline(*Pt, centerline_Param); //calculate the distance between the boundary point and centerline

		if (dtemp < dmin2){
			dmin2 = dtemp;
			idx2 = i;

		}


	}



	/* determine the midpoint of the centerline  */

	extreme_Pt1 = (CvPoint*)cvGetSeqElem(rough,idx1);
	extreme_Pt2 = (CvPoint*)cvGetSeqElem(rough,idx2);

	Midpoint->x = (extreme_Pt1->x + extreme_Pt2->x)/2;
	Midpoint->y = (extreme_Pt1->y + extreme_Pt2->y)/2;



	/** for some unknown reason, labview output centroid x and y are swapped **/


	if (Midpoint->x >= maxLoc->x) {

		centroid_out->x = maxLoc->y + shift*vy + ty;
		centroid_out->y = maxLoc->x + shift*vx + tx;

	}

	else{

		centroid_out->x = maxLoc->y - shift*vy + ty;
		centroid_out->y = maxLoc->x - shift*vx + tx;

	}


	//centroid_out->x = Midpoint->y + ty;
	//centroid_out->y = Midpoint->x + tx;


	//free(Pt);
	//free(extreme_Pt1);
	//free(extreme_Pt2);
	//free(Midpoint);

	cvReleaseImage(&TempImage2);
	cvReleaseMemStorage(&MemStorage);







	if (setROI){
			
		cvResetImageROI(CVImageDst);
		cvResetImageROI(CVImageSrc);
	}



	return 0;

}







void LongestContour(CvSeq* contours, CvSeq** ContourOfInterest){

	CvSeq* biggestContour;
	//printf("---Finding Longest Contour---\n");
	int biggest=0;
		for (contours; contours!=NULL; contours=contours->h_next){
		//printf("%d elements\n",contours->total);
		if (contours->total > biggest){
			biggest=contours->total;
			biggestContour=contours;
			//printf("Currently the biggest!\n");
		}
	}

	*ContourOfInterest=cvCloneSeq(biggestContour);
}


int Max(int a, int b){

	if (a>=b)
		
		return a;

	else

		return b;
	
}


float GetdistFromCenterline(CvPoint Pt, float centerline_Param[4]){

	float vx,vy,x0,y0,ux,uy,dot,D;
	int x1,y1;

	vx = centerline_Param[0];
	vy = centerline_Param[1];
	x0 = centerline_Param[2];
	y0 = centerline_Param[3];


	x1 = Pt.x - x0; 
	y1 = Pt.y - y0;
	D = sqrt(x1*x1+y1*y1);
	ux = x1/D;
	uy = y1/D;
	dot = ux*vx+uy*vy;
	return D*sqrt(1-dot*dot);



}
