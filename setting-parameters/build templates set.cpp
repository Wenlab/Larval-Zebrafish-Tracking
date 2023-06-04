#include "find_fish_position_dll.h"
void getAllFiles(string path, vector<string>& files);

void main()
{
	LARGE_INTEGER timeStart;
	LARGE_INTEGER timeEnd,timeEnd1,timeEnd2,timeEnd3,timeEnd4,timeEnd5,timeEnd6,timeEnd7;
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	double quadpart = (double)frequency.QuadPart;
	//QueryPerformanceCounter(&timeStart);

	//IplImage *CVImageSrc;
	//IplImage *CVImageDst;
	//IplImage *CVImageBkg;
	//IplImage *Template;
	Mat Src;
	Mat Dst;
	Mat Bkg;
	Mat Template0;
	vector<Mat> Templates;
	const int num_templates = 72;
	Point template_centroid = Point(230, 192);
	const Size template_size = Size(78,78);
	int LVWidth = 520;
	int LVHeight = 384;
	bool setROI = false;
	int BinThresh = 69;
	int Esize1 = 1;
	int Esize2 = 1;
	int width = 120;
	int height = 120;
	int shift = 12;//from the centroid of template to the centroid of brain
	Point *centroid_in;
	Point *centroid_out;
	int counting_thresh = 129;

	////const char* filename = "020180417_210945.311.jpg";
	//string filename_src = "fish_tracking_test.jpg";
	////CVImageSrc = cvLoadImage(filename, CV_LOAD_IMAGE_GRAYSCALE);
	//Src = imread(filename_src, CV_LOAD_IMAGE_GRAYSCALE);
	////namedWindow("MyWindow", CV_WINDOW_AUTOSIZE);
	////cvShowImage("MyWindow", CVImageSrc);
	////waitKey(0);
	////const char* filename_template = "fish_tracking_test_template.jpg";
	string filename_template = "fish_tracking_test_template.jpg";
	//Template = cvLoadImage(filename_template, CV_LOAD_IMAGE_GRAYSCALE);
	Template0 = imread(filename_template, CV_LOAD_IMAGE_GRAYSCALE);
	//namedWindow("Template", CV_WINDOW_AUTOSIZE);
	//imshow("Template", Template0);
	//waitKey(0);
	double angle;
	Mat rot_mat(2, 3, CV_32FC1);
	rot_mat = getRotationMatrix2D(template_centroid, 90, 1);
	warpAffine(Template0, Template0, rot_mat, Template0.size());
	for (int i = 0; i < num_templates; i++)
	{
		angle = i * 360.0 / num_templates;
		rot_mat = getRotationMatrix2D(template_centroid, angle, 1);
		Templates.push_back(Mat::zeros(template_size, CV_8UC1));
		Mat Template;
		Mat Template_ROI;
		warpAffine(Template0, Template, rot_mat, Template0.size());
		Template_ROI = Template(Rect(template_centroid.x - template_size.width / 2, template_centroid.y - template_size.height / 2, template_size.width, template_size.height));
		Template_ROI.copyTo(Templates[i]);
		//namedWindow("Template", CV_WINDOW_AUTOSIZE);
		//imshow("Template", Templates[i]);
		//waitKey(0);
		char filename[25];
		_itoa(i, filename, 10);
		string filename_full = filename;
		filename_full += ".jpg";
		imwrite(filename_full, Template_ROI);
	}

	CvSize ImageSize;
	ImageSize = cvSize(LVWidth, LVHeight);
	//CVImageDst = cvCreateImage(ImageSize, IPL_DEPTH_8U, 1);

	////const char* filename_bkg = "fish_tracking_background.jpg";
	////CVImageBkg = cvLoadImage(filename_bkg, CV_LOAD_IMAGE_GRAYSCALE);
	//Bkg = Mat::zeros(ImageSize, CV_8UC1);
	////namedWindow("MyWindow0", CV_WINDOW_AUTOSIZE);
	////cvShowImage("MyWindow0", CVImageBkg);
	////waitKey(0);

	centroid_in = (Point*)malloc(sizeof(Point));
	*centroid_in = Point(260, 200);

	centroid_out = (Point*)malloc(sizeof(Point));

	//find_centroid(Src,
	//	Dst,
	//	Bkg,
	//	Templates,
	//	LVWidth, LVHeight, setROI,
	//	BinThresh, Esize1, Esize2,
	//	width, height, shift,
	//	centroid_in, centroid_out, counting_thresh);

	//load the source image of each frame, find fish position and update the background.
	string SRC_DIR = "src_img";
	vector<string> files;
	getAllFiles(SRC_DIR, files);
	vector<string>::iterator it;
	int index_img = 0;
	for (it = files.begin(); it != files.end(); it++)
	{
		//load the source image of each frame
		Src = imread(*it, CV_LOAD_IMAGE_GRAYSCALE);
		Dst = Mat::zeros(ImageSize, CV_8UC1);

		//crop the background for find_centroid
		//namedWindow("Bkg_Global", CV_WINDOW_KEEPRATIO);
		//imshow("Bkg_Global", Bkg_Global);
		//if (index_img==1076)
		//	waitKey(0);
		//Mat Bkg_ROI;
		//Bkg_ROI = Bkg_Global(Rect(positions[index_img].x - ImageSize.width / 2, positions[index_img].y - ImageSize.height / 2, ImageSize.width, ImageSize.height));
		//Bkg_ROI.copyTo(Bkg);
		//Bkg = Bkg_ROI.clone();
		//cvSetImageROI(CVImageBkg_Global, cvRect(positions[index_img].x, positions[index_img].y, ImageSize.width, ImageSize.height));
		//cvCopy(CVImageBkg_Global, CVImageBkg, 0);

		//QueryPerformanceCounter(&timeStart);
		//find fish position
		find_centroid(Src,
			Dst,
			Bkg,
			Templates,
			LVWidth, LVHeight, setROI,
			BinThresh, Esize1, Esize2,
			width, height, shift,
			centroid_in, centroid_out, counting_thresh);
		//QueryPerformanceCounter(&timeEnd);
		//double elapsed = (timeEnd.QuadPart - timeStart.QuadPart) / quadpart;
		//cout << elapsed << "s" << endl;

		////update the background
		//Mat Bkg_new;
		//addWeighted(Src, Update_Ratio_Bkg, Bkg, 1 - Update_Ratio_Bkg, 0.0, Bkg_new);
		////namedWindow("Bkg_new", CV_WINDOW_AUTOSIZE);
		////imshow("Bkg_new", Bkg_new);
		////waitKey(0);
		//Bkg.copyTo(Bkg_new, Mask);
		////imshow("Bkg_new", Bkg_new);
		////waitKey(0);
		//Bkg_new.copyTo(Bkg_ROI);
		//namedWindow("Bkg_ROI", CV_WINDOW_AUTOSIZE);
		//imshow("Bkg_ROI", Bkg_ROI);
		////waitKey(0);
		////*CVImageBkg = IplImage(Bkg_new);
		////cvCopy(CVImageBkg,CVImageBkg_Global, 0);
		////cvResetImageROI(CVImageBkg_Global);
		//namedWindow("Bkg_Global", CV_WINDOW_KEEPRATIO);
		//imshow("Bkg_Global", Bkg_Global);
		////waitKey(0);
		if (index_img % 1 == 0)
			waitKey(0);
		index_img++;
		//destroyAllWindows();
	}

	//cvReleaseImage(&CVImageSrc);
	//cvReleaseImage(&CVImageDst);
	//cvReleaseImage(&CVImageBkg);
	free(centroid_in);
	free(centroid_out);
}

void getAllFiles(string path, vector<string>& files)
{
	long   hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					files.push_back(p.assign(path).append("/").append(fileinfo.name));
					getAllFiles(p.assign(path).append("/").append(fileinfo.name), files);
				}
			}
			else
			{
				files.push_back(p.assign(path).append("/").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}