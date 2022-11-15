//CUDA 11.1
//TensorRT 7.2.2.3

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <numeric>

#include <NvInfer.h>
#include <NvOnnxParser.h>
#include <NvInferPlugin.h>
#include <cuda_runtime.h>


//opencv
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<opencv2/imgcodecs/imgcodecs.hpp>
#include<opencv2/imgproc.hpp>


using namespace nvinfer1;
using namespace nvonnxparser;

using std::cout;
using std::endl;
using std::vector;

using cv::Mat;
using cv::Point;

#define IMG_SIZE 320

class Logger : public nvinfer1::ILogger {
public:
	void log(Severity severity, const char *msg) override {
		using namespace std;
		string s;
		switch (severity) {
		case Severity::kINTERNAL_ERROR:
			s = "INTERNAL_ERROR";
			break;
		case Severity::kERROR:
			s = "ERROR";
			break;
		case Severity::kWARNING:
			s = "WARNING";
			break;
		case Severity::kINFO:
			s = "INFO";
			break;
		case Severity::kVERBOSE:
			s = "VERBOSE";
			break;
		}
		cerr << s << ": " << msg << endl;
	}
};

// Using unique_ptr with Destroy is optional, but beats calling destroy() for everything
// Borrowed from the NVidia tutorial, nice C++ skills !
template<typename T>
struct Destroy {
	void operator()(T *t) const {
		t->destroy();
	}
};


class TRTruntime
{
public:
	TRTruntime();
	TRTruntime(int bs, int h, int w, int hmN);
	~TRTruntime();


	// Parse onnx file and create a TRT engine
	void createCudaEngine(const std::string &onnxFileName);

	void SerializeModel(const std::string & trtFileName);
	void DeserializeModel(const std::string & trtFileName);

	void createInferenceContext();

	// Run a single inference
	void launchInference(Mat img, std::vector<Point>& outputTensor);
	void launchInference(Mat img, std::vector<Point>& outputTensor, std::vector<float>& confidence);
private:

	Logger logger;
	ICudaEngine *engine;
	IExecutionContext *context;
	cudaStream_t stream;
	
	
	void *bindings[2]{ 0 };
	int batchSize;
	int imgH;
	int imgW;
	int heatmapNum;

};



