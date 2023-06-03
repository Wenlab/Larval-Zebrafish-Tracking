#include"TRTruntime.h"
#include"Timer.h"
#include"getopt.h"

#include<io.h>
#include <sstream>

using namespace std;
using namespace cv;

using namespace nvinfer1;
using namespace nvonnxparser;


void getFileNames(std::string path, std::vector<std::string>& files);

int main()
{
	TRTruntime trt(1, 360, 360, 2);

	trt.createCudaEngine("trackKeyPointModel_0331_unet_360.onnx");
	trt.SerializeModel("trackKeyPointModel_0331_unet_360.trt");
	trt.DeserializeModel("trackKeyPointModel_0331_unet_360.trt");
	trt.createInferenceContext();


	std::vector<std::string> imgfilenames;
	std::string path = "F:/Kexin/dl-tracking/tracking-CNN/detection/dataset/keyPoint220304_small/val/img";
	getFileNames(path, imgfilenames);

	vector<Mat> inputTensor;
	vector<cv::Point> outputTensor;
	for (int j = 0; j < 10; j++)
	{
		cout << imgfilenames[j] << endl;
		Mat inputImg = imread(imgfilenames[j], cv::IMREAD_GRAYSCALE);
		if (inputImg.empty())
		{
			cout << "can't read image!!!" << endl;
			break;
		}
		//imshow("test", inputImg);
		//waitKey(0);
		//cv::resize(inputImg, inputImg, cv::Size(360, 360));
		Mat inputImg2;
		inputImg.convertTo(inputImg2, CV_32FC1);
		//inputImg2 = inputImg2 / 255;
		cout << inputImg2.rows << endl;
		inputTensor.push_back(inputImg2.clone());

		trt.launchInference(inputTensor, outputTensor);

		for (int i = 0; i < outputTensor.size(); i++)
		{
			cv::circle(inputImg, outputTensor[i], 3, 128, 2);
		}
		cv::imshow("test", inputImg);
		cv::waitKey(0);
		inputTensor.clear();
		outputTensor.clear();
	}
	cv::destroyWindow("test");


	//	// test speed

	Mat inputImg = imread(imgfilenames[1], cv::IMREAD_GRAYSCALE);
	//cv::resize(inputImg, inputImg, cv::Size(256, 256));
	Mat inputImg2;
	inputImg.convertTo(inputImg2, CV_32FC1);
	inputImg2 = inputImg2 / 255;
	inputTensor.push_back(inputImg2);

	Timer timer;
	for (int j = 0; j < 10; j++)
	{
		timer.start();
		for (int i = 0; i < 1000; i++)
		{
			trt.launchInference(inputTensor, outputTensor);
		}
		timer.stop();
		cout << "runtime: " << timer.getElapsedTimeInMilliSec() / 1000 << " ms" << endl;
	}


	return 0;
}


void getFileNames(std::string path, std::vector<std::string>& files)
{
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFileNames(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}