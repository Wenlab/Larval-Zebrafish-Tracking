import torch
import numpy as np
import onnxruntime as ort

import time

import dataloader as dl

from test import saveHeatmapResultImg

import os
import dataloader as dl

if __name__ == "__main__":

    device=torch.device("cuda:2" if torch.cuda.is_available() else "cpu")

    ort_session = ort.InferenceSession("trackKeyPointModel_0304_simple_fixbatch.onnx", 
                    providers=['CUDAExecutionProvider']) # 

    savepath='./dataset/keyPoint220304_small/test/checkonnx/'
    if not os.path.exists(savepath):
        os.makedirs(savepath)

    testPath="./dataset/keyPoint220304_small/test/"
    test_loader=dl.TrackingKeyPointDataset(testPath)
    batchSize=1
    test_dataloader = torch.utils.data.DataLoader(test_loader, batch_size = batchSize, shuffle = False, num_workers = 1)

    for i, data in enumerate(test_dataloader):
        with torch.set_grad_enabled(False):
            img,label = data[0].to(device), data[1].to(device)
            outputs = ort_session.run(None,{ 'input' : img.cpu().numpy()})
            saveHeatmapResultImg(img,torch.from_numpy(outputs[0]),savepath,i)

    y_size = 256
    x_size = 256
    no_channels = 1
    batch_size = 1
    example_source = np.random.randn(batch_size, no_channels, y_size, x_size).astype(np.float32)

    for j in range(100):
        T1 = time.clock()
        for i in range(1000):
            outputs = ort_session.run(None,{ 'input' : example_source}) 
        T2 = time.clock()
        print('模型运行1000次:%s毫秒' % ((T2 - T1)*1000))
        print('平均时间:%s毫秒' % ((T2 - T1)))
