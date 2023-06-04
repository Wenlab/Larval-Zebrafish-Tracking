#ifdef FIND_FISH_POSITION_DLL_EXPORTS
#define FIND_FISH_POSITION_DLL_API __declspec(dllexport)
#else
#define FIND_FISH_POSITION_DLL_API __declspec(dllimport)
#endif

#include "opencv2/opencv.hpp"
#include "opencv2/core/ocl.hpp"
using namespace cv;
using namespace std;

#define FISH_FOUND 0
#define FISH_NOT_FOUND -1

//LVWidth, LVHeight: size of source image
//BkgWidth, BkgHeight: size of global background
//BkgWidth = 4*LVWidth; BkgHeight = 4*LVHeight;
//templates: 
//n_templates: number of templates
//TemplatesSize: 
//width, height: size of ROI
//scale_x, scale_y, theta: scale position to image coordinate, rotation angle from stage coordinate to image coordinate
//scale_x = 750.0 / (10000 * 20); scale_y = 750.0 / (12800 * 20); theta = atan(0.06);
//The stage should be initially at the center to avoid overflow.
//BinThresh: binary threshhold
//BinThresh = 100;
//shift: shift from centroid of template to centroid of head
//counting_thresh: if the number of white points in the image after threshold is smaller than counting_thresh, no fish here.
//width_fish,length_fish: for calculating the mask. while the source image size is 1040*768, the fish is about 300*60.
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
	int shift, Point *centroid_in, Point *centroid_out, Point2d *heading_vector,
	double Update_Ratio_Bkg);

void transform_coordination(Point *position, int BkgWidth, int BkgHeight, double scale_x, double scale_y, double theta);

void crop_background(Mat &Bkg_global, Mat &Bkg, Point* position, 
	int LVWidth, int LVHeight);

int update_background(Mat &Bkg, Mat &Src, Rect Mask, double Update_Ratio_Bkg);

int TargetContour(vector<vector <Point>> contours, Point refPt,int minArea, int *matching_index,Point *targetCentroid);

int Max(int a, int b);

float GetdistFromCenterline(Point Pt, Vec4f centerline_Param);

double dot_product(Point2d v1, Point2d v2);