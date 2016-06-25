#ifndef FIND_FISH_POSITION_DLL_H_
#define	FIND_FISH_POSITION_DLL_H_

#ifdef __cplusplus
    extern "C" {
#endif


#include <NIVision.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#ifdef BUILDING_FIND_FISH_POSITION_DLL
#define FIND_FISH_POSITION_DLL __declspec(dllexport)
#else
#define FIND_FISH_POSITION_DLL __declspec(dllimport)
#endif

#define NSIZEX 1040
#define NSIZEY 768


int FIND_FISH_POSITION_DLL find_centroid(char* LVImagePtrSrc, int LVLineWidthSrc,
			      						char* LVImagePtrDst, int LVLineWidthDst,
				  						int LVWidth, int LVHeight, bool setROI,
				  						int BinThresh, int Esize1, int Esize2, 
				  						int width, int height, int shift,
				  						Point *centroid_in, Point *centroid_out);

void LongestContour(CvSeq* contours, CvSeq** ContourOfInterest);

int Max(int a, int b);


#ifdef __cplusplus
}
#endif

#endif /* FIND_FISH_POSITION_DLL_H_ */

