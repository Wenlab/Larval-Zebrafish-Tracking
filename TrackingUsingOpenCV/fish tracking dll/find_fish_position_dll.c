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




	if (setROI) {

		
		CvRect ROI;


		tx = ((centroid_in->x-width/2) > 0) ? centroid_in->x-width/2 : 0;
		ty = ((centroid_in->y-height/2) > 0) ? centroid_in->y-width/2 : 0;

		ROI_width = ((centroid_in->x+width/2) < LVWidth) ? width : (LVWidth-centroid_in->x)*2;
		ROI_height = ((centroid_in->y+height/2) < LVHeight) ? height : (LVHeight-centroid_in->y)*2;


		ROI = cvRect(tx,ty,ROI_width,ROI_height);

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
	*maxLoc=cvPoint(centroid_in->x,centroid_in->y);
	cvMinMaxLoc(TempImage,&min,&max,NULL,maxLoc,NULL);
		
	
	cvReleaseImage(&TempImage);

	//end_t = clock(); 

	//total_t=((double) end_t-start_t) / CLOCKS_PER_SEC;

	//total_t = end_t-start_t;

	//return (int) total_t*1000;

	

	/** Find Contours and use contours to find fish center**/
	

	CvSeq* contours;
	CvPoint* Fish_Center = (CvPoint*) malloc(sizeof(CvPoint));
	
	cvCopy(CVImageDst,TempImage2);

	CvMemStorage* MemStorage=cvCreateMemStorage(0);

	cvFindContours(TempImage2,MemStorage, &contours,sizeof(CvContour),CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE,cvPoint(0,0));


	CvSeq* rough;
	/** Find Longest Contour **/
	if (contours) LongestContour(contours,&rough);

	else{

	 	centroid_out->x=maxLoc->x+tx;
		centroid_out->y=maxLoc->y+ty;
		return 1;
	}

	
	int TotalBpts;
	int i;
	int sum_x=0;
	int sum_y=0;
	int D;

	CvPoint* Pt = (CvPoint*) malloc(sizeof(CvPoint));
	TotalBpts = rough->total;
		
	for (i=0; i<TotalBpts; i++) {
			
		Pt = (CvPoint*)cvGetSeqElem(rough,i);
		sum_x+=Pt->x;
		sum_y+=Pt->y;
	}

	Fish_Center->x=sum_x/TotalBpts;
	Fish_Center->y=sum_y/TotalBpts;

	

	D = (int) sqrt (pow(Fish_Center->x - maxLoc->x,2) + pow(Fish_Center->y - maxLoc->y,2) );
	centroid_out->x = shift*(Fish_Center->x - maxLoc->x)/D + maxLoc->x;
	centroid_out->y = shift*(Fish_Center->y - maxLoc->y)/D + maxLoc->y; 

	

	centroid_out->x = centroid_out->x + tx;
	centroid_out->y = centroid_out->y + ty;

	
	//free(Pt);
	//free(maxLoc);
	//free(Fish_Center);

	cvReleaseImage(&TempImage2);
	cvReleaseMemStorage(&MemStorage);



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
