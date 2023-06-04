#include "find_fish_position_dll.h"
void getAllFiles(string path, vector<string>& files);

void main()
{
	LARGE_INTEGER timeStart;
	LARGE_INTEGER timeEnd,timeEnd1,timeEnd2,timeEnd3,timeEnd4,timeEnd5,timeEnd6,timeEnd7;
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	double quadpart = (double)frequency.QuadPart;

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

	string filename_template = "fish_tracking_test_template.jpg";
	Template0 = imread(filename_template, CV_LOAD_IMAGE_GRAYSCALE);
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
		char filename[25];
		_itoa(i, filename, 10);
		string filename_full = filename;
		filename_full += ".jpg";
		imwrite(filename_full, Template_ROI);
	}

	CvSize ImageSize;
	ImageSize = cvSize(LVWidth, LVHeight);

	centroid_in = (Point*)malloc(sizeof(Point));
	*centroid_in = Point(260, 200);

	centroid_out = (Point*)malloc(sizeof(Point));
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

		find_centroid(Src,
			Dst,
			Bkg,
			Templates,
			LVWidth, LVHeight, setROI,
			BinThresh, Esize1, Esize2,
			width, height, shift,
			centroid_in, centroid_out, counting_thresh);

		if (index_img % 1 == 0)
			waitKey(0);
		index_img++;
	}

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