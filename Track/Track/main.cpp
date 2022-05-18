//// By Oleksiy Grechnyev, IT-JIM
//// Example 1: An (almost) minimal TensorRT C++ inference example
//// This one uses model1.onnx with fixed batch size (1)
//// Batch size at inference must be the same !
//
//#include <fstream>
//#include <iostream>
//#include <memory>
//#include <string>
//#include <vector>
//#include <numeric>
//
//#include <NvInfer.h>
//#include <NvOnnxParser.h>
//#include <NvInferPlugin.h>
//#include <cuda_runtime.h>
//
//
////opencv
//#include<opencv2/highgui/highgui.hpp>
//#include<opencv2/opencv.hpp>
//#include<opencv2/imgcodecs/imgcodecs.hpp>
//#include<opencv2/imgproc.hpp>
//
//#include"Timer.h"
//
//using namespace nvinfer1;
//using namespace nvonnxparser;
//
//using std::cout;
//using std::endl;
//using std::vector;
//
//using cv::Mat;
//using cv::Point;
//
////======================================================================================================================
//
//class Logger : public nvinfer1::ILogger {
//public:
//	void log(Severity severity, const char *msg) override {
//		using namespace std;
//		string s;
//		switch (severity) {
//		case Severity::kINTERNAL_ERROR:
//			s = "INTERNAL_ERROR";
//			break;
//		case Severity::kERROR:
//			s = "ERROR";
//			break;
//		case Severity::kWARNING:
//			s = "WARNING";
//			break;
//		case Severity::kINFO:
//			s = "INFO";
//			break;
//		case Severity::kVERBOSE:
//			s = "VERBOSE";
//			break;
//		}
//		cerr << s << ": " << msg << endl;
//	}
//};
////======================================================================================================================
//
///// Using unique_ptr with Destroy is optional, but beats calling destroy() for everything
///// Borrowed from the NVidia tutorial, nice C++ skills !
//template<typename T>
//struct Destroy {
//	void operator()(T *t) const {
//		t->destroy();
//	}
//};
//
////======================================================================================================================
//
///// Parse onnx file and create a TRT engine
//nvinfer1::ICudaEngine *createCudaEngine(const std::string &onnxFileName, nvinfer1::ILogger &logger) 
//{
//	using namespace std;
//	using namespace nvinfer1;
//
//	unique_ptr<IBuilder, Destroy<IBuilder>> builder{ createInferBuilder(logger) };
//
//	unique_ptr<INetworkDefinition, Destroy<INetworkDefinition>> network{
//			builder->createNetworkV2(1U << (unsigned)NetworkDefinitionCreationFlag::kEXPLICIT_BATCH) };
//
//	unique_ptr<nvonnxparser::IParser, Destroy<nvonnxparser::IParser>> parser{
//			nvonnxparser::createParser(*network, logger) };
//
//	//if (!parser->parseFromFile(onnxFileName.c_str(), static_cast<int>(ILogger::Severity::kERROR)))
//	//	throw runtime_error("ERROR: could not parse ONNX model " + onnxFileName + " !");
//	parser->parseFromFile(onnxFileName.c_str(), static_cast<int>(ILogger::Severity::kWARNING));
//
//	// Modern version with config
//	unique_ptr<IBuilderConfig, Destroy<IBuilderConfig>> config(builder->createBuilderConfig());
//	// This is needed for TensorRT 6, not needed by 7 !
//	config->setMaxWorkspaceSize(64 * 1024 * 1024);
//
//	//设置模型精度
//	config->setFlag(BuilderFlag::kFP16);
//
//	return builder->buildEngineWithConfig(*network, *config);
//}
//
////======================================================================================================================
//// Run a single inference
//void launchInference(nvinfer1::IExecutionContext *context, cudaStream_t stream, std::vector<Mat> const &inputTensor,
//	std::vector<cv::Point> &outputTensor, void **bindings, int batchSize) 
//{
//
//	int inputId = 0, outputId = 1; // Here I assume input=0, output=1 for the current network
//
//	// Infer synchronously as an alternative, no stream needed
////    cudaMemcpy(bindings[inputId], inputTensor.data(), inputTensor.size() * sizeof(float), cudaMemcpyHostToDevice);
////    bool res = context->executeV2(bindings);
////    cudaMemcpy(outputTensor.data(), bindings[outputId], outputTensor.size() * sizeof(float), cudaMemcpyDeviceToHost);
//
//	// Infer asynchronously, in a proper cuda way !
//	void *data = malloc(1 * 256 * 256 * sizeof(float));
//	memcpy(data, inputTensor[0].ptr<float>(0), 1 * 256 * 256 * sizeof(float));
//	//memcpy(data, inputTensor.data(), 52 * 77 * 95 * sizeof(float));
//	cudaMemcpyAsync(bindings[inputId], data, 1 * 256 * 256 * sizeof(float), cudaMemcpyHostToDevice,
//		stream);
//
//
//	float prob[1 * 2 * 256 * 256];
//	context->enqueueV2(bindings, stream, nullptr);
//	//cout << "infer succesesful!!" << endl;
//	cudaMemcpyAsync(prob, bindings[outputId], 1 * 2 * 256 * 256 * sizeof(float),
//		cudaMemcpyDeviceToHost, stream);
//
//
//	int heatmapNum = 2;
//	std::vector<float> results(std::begin(prob), std::end(prob));
//	/* get result  */
//	for (int i = 0; i < heatmapNum; i++)
//	{
//		vector<float>::const_iterator First = results.begin() + i * 256 * 256; // 找到第二个迭代器
//		vector<float>::const_iterator Second = results.begin() + (i + 1) * 256 * 256; // 找到第三个迭代器
//		vector<float> result(First, Second);
//
//		auto maxPosition1 = std::max_element(result.begin(), result.end());
//		//cout << *maxPosition1 << " at the postion of " << maxPosition1 - result.begin() << endl;
//
//		int pos = maxPosition1 - result.begin();
//		cv::Point maxLoc(pos % 256, floor(pos / 256));
//		outputTensor.push_back(maxLoc);
//
//		//cv::Mat heatmap(cv::Size(256, 256), CV_32FC1, result.data());
//		//outputTensor.push_back(heatmap.clone());
//		//result.clear();
//	}
//
//	//cv::Mat check = outputTensor[0] - outputTensor[1];
//	//cout<<cv::countNonZero(check)<<endl;
//
//	//std::vector<float> results(std::begin(prob), std::end(prob));
//	//cout << results.size() << endl;
//
//	//std::vector<cv::Point> maxLoc;
//	//std::vector<float> maxNum;
//
//	//auto maxPosition1 = std::max_element(results.begin(), results.begin() + 256 * 256);
//	//auto maxPosition2 = std::max_element(results.begin() + 256 * 256, results.end());
//
//	//cout << *maxPosition1 << " at the postion of " << maxPosition1 - results.begin() << endl;
//	//cout << *maxPosition2 << " at the postion of " << maxPosition2 - results.begin() + 256 * 256 << endl;
//
//
//
//	////cout << outputMap << endl;
//	//cv::split(outputMap, outputTensor);
//
//	//cout << "get output successful!!!" << endl;
//}
//
////======================================================================================================================
//int main() {
//	using namespace std;
//	using namespace nvinfer1;
//
//	//// Parse model, create engine
//	Logger logger;
//	//logger.log(ILogger::Severity::kINFO, "Creating engine ...");
//	//unique_ptr<ICudaEngine, Destroy<ICudaEngine>> engine(createCudaEngine("affineNet_0105_0824_0924_CMTK_AM_TM3_x10.onnx", logger));
//
//	//if (!engine)
//	//	throw runtime_error("Engine creation failed !");
//
//
//
//	//// for test and DEBUG
//	////Optional : Print all bindings : name + dims + dtype
//	//cout << "=============\nBindings :\n";
//	//int n = engine->getNbBindings();
//	//for (int i = 0; i < n; ++i) {
//	//	Dims d = engine->getBindingDimensions(i);
//	//	cout << i << " : " << engine->getBindingName(i) << " : dims=";
//	//	for (int j = 0; j < d.nbDims; ++j) {
//	//		cout << d.d[j];
//	//		if (j < d.nbDims - 1)
//	//			cout << "x";
//	//	}
//	//	cout << " , dtype=" << (int)engine->getBindingDataType(i) << " ";
//	//	cout << (engine->bindingIsInput(i) ? "IN" : "OUT") << endl;
//	//}
//	//cout << "=============\n\n";
//
//	////serialize Model
//	//IHostMemory *gieModelStream = engine->serialize();
//	//std::string serialize_str;
//	//serialize_str.resize(gieModelStream->size());
//	//memcpy((void*)serialize_str.data(), gieModelStream->data(), gieModelStream->size());
//
//	//std::ofstream serialize_output_stream("affineNet_0105_0824_0924_CMTK_AM_TM3_x10.trt", std::ios_base::out | std::ios_base::binary);
//	//serialize_output_stream << serialize_str;
//	//serialize_output_stream.close();
//	//cout << "serialize model successful!!!" << endl;
//
//
//	//deserialize model from file
//	std::string cached_path = "trackKeyPointModel_0304_simple4_fp16.trt";
//	std::ifstream fin(cached_path, std::ios_base::in | std::ios_base::binary);
//	fin.seekg(0, ios::end);
//	streampos size = fin.tellg();
//	fin.seekg(0, ios::beg);
//
//	char * buff = new char[size];
//	fin.read(buff, size);
//	fin.close();
//	IRuntime* runtime = createInferRuntime(logger);
//	initLibNvInferPlugins(&logger, "");   //必须加上这一句才能反序列化成功，加载官方自定义的层
//	ICudaEngine* re_engine = runtime->deserializeCudaEngine((void *)buff, size, NULL);
//	delete buff;
//	cout << "deserialize model successful!!!" << endl;
//
//	// Create context
//	unique_ptr<IExecutionContext, Destroy<IExecutionContext>> context(re_engine->createExecutionContext());
//
//	// Create data structures for the inference
//	cudaStream_t stream;
//	cudaStreamCreate(&stream);
//
//
//	void *bindings[2]{ 0 };
//	int batchSize = 1;
//	// Alloc cuda memory for IO tensors
//	for (int i = 0; i < re_engine->getNbBindings(); ++i) 
//	{
//		Dims dims{ re_engine->getBindingDimensions(i) };
//		size_t size = accumulate(dims.d, dims.d + dims.nbDims, batchSize, multiplies<size_t>());
//		// Create CUDA buffer for Tensor.
//		cudaMalloc(&bindings[i], size * sizeof(float));
//	}
//
//	// Run the inference !
//	cout << "Running the inference !" << endl;
//
//
//	vector<Mat> inputTensor;
//	vector<cv::Point> outputTensor;
//
//	Mat inputImg = imread("004261.png", cv::IMREAD_GRAYSCALE);
//	cv::resize(inputImg, inputImg, cv::Size(256, 256));
//	Mat inputImg2;
//	inputImg.convertTo(inputImg2, CV_32FC1);
//	inputImg2 = inputImg2 / 255;
//	inputTensor.push_back(inputImg2);
//	////cout << inputImg2.channels() << endl;
//	
//	// test result
//	launchInference(context.get(), stream, inputTensor, outputTensor, bindings, batchSize);
//	cudaStreamSynchronize(stream);
//	for (int i = 0; i < outputTensor.size(); i++)
//	{
//		cv::circle(inputImg, outputTensor[i], 3, 128, 2);
//	}
//	cv::imshow("test",inputImg);
//	cv::waitKey(0);
//	cv::destroyWindow("test");
//
//	// test speed
//	Timer timer;
//	for (int j = 0; j < 10; j++)
//	{
//		timer.start();
//		for (int i = 0; i < 1000; i++)
//		{
//			launchInference(context.get(), stream, inputTensor, outputTensor, bindings, batchSize);
//			cudaStreamSynchronize(stream);
//		}
//		timer.stop();
//		cout << "runtime: " << timer.getElapsedTimeInMilliSec() / 1000 << " ms" << endl;
//	}
//
//
//	cudaStreamDestroy(stream);
//	cudaFree(bindings[0]);
//	cudaFree(bindings[1]);
//	return 0;
//}