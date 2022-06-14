#include "TRTruntime.h"

using namespace nvinfer1;
using namespace nvonnxparser;

using std::cout;
using std::endl;
using std::vector;

using cv::Mat;
using cv::Point;

TRTruntime::TRTruntime()
{
}

TRTruntime::TRTruntime(int bs, int h, int w, int hmN)
{
	batchSize = bs;
	imgH = h;
	imgW = w;
	heatmapNum = hmN;
}

TRTruntime::~TRTruntime()
{
	cudaStreamDestroy(stream);
	cudaFree(bindings[0]);
	cudaFree(bindings[1]);
}

void TRTruntime::createCudaEngine(const std::string & onnxFileName)
{
	IBuilder *builder{ createInferBuilder(logger) };
	INetworkDefinition *network{
			builder->createNetworkV2(1U << (unsigned)NetworkDefinitionCreationFlag::kEXPLICIT_BATCH) };
	nvonnxparser::IParser *parser{
			nvonnxparser::createParser(*network, logger) };

	parser->parseFromFile(onnxFileName.c_str(), static_cast<int>(ILogger::Severity::kWARNING));

	////如果设备支持INT8精度，则使用INT8精度
	//if (builder->platformHasFastInt8())
	//{
	//	builder->setInt8Mode(true);
	//	builder->setInt8Mode(dataType == DataType::kINT8);
	//	builder->setInt8Calibrator(calibrator);//校准器接口
	//};


	IBuilderConfig*  config(builder->createBuilderConfig());
	config->setMaxWorkspaceSize(64 * 1024 * 1024);
	config->setFlag(BuilderFlag::kFP16);   //设置量化类型

	engine = builder->buildEngineWithConfig(*network, *config);

	if (!engine)
		cout << "Engine creation failed " << endl;

	return ;
}

void TRTruntime::SerializeModel(const std::string & trtFileName)
{
	IHostMemory *gieModelStream = engine->serialize();
	std::string serialize_str;
	serialize_str.resize(gieModelStream->size());
	memcpy((void*)serialize_str.data(), gieModelStream->data(), gieModelStream->size());

	std::ofstream serialize_output_stream(trtFileName, std::ios_base::out | std::ios_base::binary);
	serialize_output_stream << serialize_str;
	serialize_output_stream.close();
	cout << "serialize model successful!!!" << endl;

	return;
}

void TRTruntime::DeserializeModel(const std::string & trtFileName)
{
	std::string cached_path = trtFileName;
	std::ifstream fin(cached_path, std::ios_base::in | std::ios_base::binary);
	fin.seekg(0, std::ios::end);
	std::streampos size = fin.tellg();
	fin.seekg(0, std::ios::beg);

	char * buff = new char[size];
	fin.read(buff, size);
	fin.close();
	IRuntime* runtime = createInferRuntime(logger);
	initLibNvInferPlugins(&logger, "");   //必须加上这一句才能反序列化成功，加载官方自定义的层
	engine = runtime->deserializeCudaEngine((void *)buff, size, NULL);
	delete buff;
	cout << "deserialize model successful!!!" << endl;


	// Optional : Print all bindings : name + dims + dtype
	cout << "=============\nBindings :\n";
	int n = engine->getNbBindings();
	for (int i = 0; i < n; ++i) {
		Dims d = engine->getBindingDimensions(i);
		cout << i << " : " << engine->getBindingName(i) << " : dims=";
		for (int j = 0; j < d.nbDims; ++j) {
			cout << d.d[j];
			if (j < d.nbDims - 1)
				cout << "x";
		}
		cout << " , dtype=" << (int)engine->getBindingDataType(i) << " ";
		cout << (engine->bindingIsInput(i) ? "IN" : "OUT") << endl;
	}
	cout << "=============\n\n";

	return;
}

void TRTruntime::createInferenceContext()
{
	context = engine->createExecutionContext();
	cudaStreamCreate(&stream);

	// Alloc cuda memory for IO tensors
	for (int i = 0; i < engine->getNbBindings(); ++i)
	{
		Dims dims{ engine->getBindingDimensions(i) };
		//1*1*256*256
		size_t size = std::accumulate(dims.d, dims.d + dims.nbDims, batchSize, std::multiplies<size_t>());
		// Create CUDA buffer for Tensor.
		cudaMalloc(&bindings[i], size * sizeof(float));
	}
	
	return;
}

void TRTruntime::launchInference(Mat img, std::vector<Point>& outputTensor)
{
	
	int inputId = 0, outputId = 1; // Here I assume input=0, output=1 for the current network
	// Infer asynchronously, in a proper cuda way !
	void *data = malloc(batchSize * imgH * imgW * sizeof(float));
	
	memcpy(data, img.ptr<float>(0), batchSize * imgH * imgW * sizeof(float));

	
	cudaMemcpyAsync(bindings[inputId], data, batchSize * imgH * imgW * sizeof(float), cudaMemcpyHostToDevice,
		stream);

	
	//cout << "1111" << endl;
	float prob[1 * 2 * IMG_SIZE * IMG_SIZE];   //如果更换模型 需要改这里的参数
	context->enqueueV2(bindings, stream, nullptr);
	//cout << "infer succesesful!!" << endl;
	cudaMemcpyAsync(prob, bindings[outputId],  batchSize * heatmapNum * imgH * imgW * sizeof(float),
		cudaMemcpyDeviceToHost, stream);

	//cout << "2222" << endl;
	
	/* get result  */
	
	std::vector<float> results(std::begin(prob), std::end(prob));
	for (int i = 0; i < heatmapNum; i++)
	{
		vector<float>::const_iterator First = results.begin() + i * imgH * imgW; // 找到heatmap的第一个像素
		vector<float>::const_iterator Second = results.begin() + (i + 1) *  imgH * imgW; // 找到heatmap的末尾
		vector<float> result(First, Second);

		auto maxPosition1 = std::max_element(result.begin(), result.end());
		int pos = maxPosition1 - result.begin();
		cv::Point maxLoc(pos % imgH, floor(pos / imgW));
		outputTensor.push_back(maxLoc);

	}
	cudaStreamSynchronize(stream);
	free(data);
	data = NULL;

	//cout << "3333" << endl;
	return;
}

void TRTruntime::launchInference(Mat img, std::vector<Point>& outputTensor, std::vector<float>& confidence)
{

	int inputId = 0, outputId = 1; // Here I assume input=0, output=1 for the current network
	// Infer asynchronously, in a proper cuda way !
	void *data = malloc(batchSize * imgH * imgW * sizeof(float));

	memcpy(data, img.ptr<float>(0), batchSize * imgH * imgW * sizeof(float));


	cudaMemcpyAsync(bindings[inputId], data, batchSize * imgH * imgW * sizeof(float), cudaMemcpyHostToDevice,
		stream);


	//cout << "1111" << endl;
	float prob[1 * 2 * IMG_SIZE * IMG_SIZE];   //如果更换模型 需要改这里的参数
	context->enqueueV2(bindings, stream, nullptr);
	//cout << "infer succesesful!!" << endl;
	cudaMemcpyAsync(prob, bindings[outputId], batchSize * heatmapNum * imgH * imgW * sizeof(float),
		cudaMemcpyDeviceToHost, stream);

	//cout << "2222" << endl;

	/* get result  */

	std::vector<float> results(std::begin(prob), std::end(prob));
	for (int i = 0; i < heatmapNum; i++)
	{
		vector<float>::const_iterator First = results.begin() + i * imgH * imgW; // 找到heatmap的第一个像素
		vector<float>::const_iterator Second = results.begin() + (i + 1) *  imgH * imgW; // 找到heatmap的末尾
		vector<float> result(First, Second);

		auto maxPosition1 = std::max_element(result.begin(), result.end());
		int pos = maxPosition1 - result.begin();
		cv::Point maxLoc(pos % imgH, floor(pos / imgW));
		outputTensor.push_back(maxLoc);
		confidence.push_back(result[pos]);

	}
	cudaStreamSynchronize(stream);
	free(data);
	data = NULL;

	//cout << "3333" << endl;
	return;
}
