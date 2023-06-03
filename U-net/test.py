import numpy as np
import time

import torch
import torch.nn as nn
import torch.optim as optim
import torch.utils
import torch.nn.functional as F

from PIL import Image,ImageDraw
import cv2

import simpleModel as trackModel

import dataloader as dl


import os

def saveResultImg(imgs,label_caculate,savepath,batchNum):
    for i in range(imgs.size(0)):
        img=imgs[i,:,:,:]
        img = torch.squeeze(img, 0)
        img=img*255
        label=label_caculate[i,:]
        label=label.cpu().numpy()
        label[0]=label[0]*672
        label[1]=label[1]*360
        label[2]=label[2]*672
        label[3]=label[3]*360
        # print(label.shape)
        img = img.cpu().numpy()
        img_cv = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
        cv2.circle(img_cv,(label[0],label[1]),2,(255, 0, 0),-1)
        cv2.circle(img_cv,(label[2],label[3]),2,(0, 255, 0),-1)

        
        cv2.imwrite(savepath+'%04d'%batchNum+'_'+'%04d'%i+'.png', img_cv)
        # img_pil.save(savepath+'%04d'%batchNum+'_'+'%04d'%i+'.png')


def saveHeatmapResultImg(imgs,heatmaps,savepath,batchNum):
    for i in range(imgs.size(0)):
        img=imgs[i,:,:,:]
        img = torch.squeeze(img, 0)
        img=img*255
        img = img.cpu().numpy()

        # print(heatmaps.size())
        hms=heatmaps[i,:,:,:]
        hms=torch.squeeze(hms, 0)
        hms=hms.cpu().numpy()
        points=getPointFromHeatmap(hms)

        img_cv = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
        for j in range(len(points)):
            cv2.circle(img_cv,(points[j][1],points[j][0]),2,(255, 0, 0),-1)

        cv2.imwrite(savepath+'%04d'%batchNum+'_'+'%04d'%i+'.png', img_cv)

        # print(hms.shape)
        # print(hms[1,:,:].max())
        # cv2.imwrite(savepath+'%04d'%batchNum+'_'+'%04d'%i+'_hp.png', hms[1,:,:])  # write out heatmap
    return 

def getPointFromHeatmap(heatmaps):
    points=[]
    for heatmap in heatmaps:
        pos=np.unravel_index(np.argmax(heatmap), heatmap.shape)
        # print(np.argmax(heatmap))
        # print(pos)
        points.append(pos)
    return points


def getDistanceError(ref,caculate):
    for i in range(ref.size(0)):
        ref_hm=ref[i,:,:,:].cpu().numpy()
        caculate_hm=caculate[i,:,:,:].cpu().numpy()
        ref_points=getPointFromHeatmap(ref_hm)
        caculate_points=getPointFromHeatmap(caculate_hm)
        dist=0
        pdist=[]
        for j in range(len(ref_points)):
            d=np.linalg.norm(np.array(ref_points[j])-np.array(caculate_points[j]))
            dist=dist+d
            pdist.append(d)
    return dist,np.array(pdist)


if __name__ == "__main__":

    device=torch.device("cuda:2" if torch.cuda.is_available() else "cpu")
    # device=torch.device("cpu")

    modelPath='./model/trackKeyPointModel_0618_unet_320crop.pt'
    model=trackModel.load_network(device, path=modelPath)
    model=model.to(device)

    loss_function=torch.nn.MSELoss()

    savepath='./dataset/keyPoint220618_320crop/test/check_unet320crop/'
    if not os.path.exists(savepath):
        os.makedirs(savepath)


    testPath="./dataset/keyPoint220618_320crop/test/"
    test_loader=dl.TrackingKeyPointDataset(testPath)
    batchSize=4
    test_dataloader = torch.utils.data.DataLoader(test_loader, batch_size = batchSize, shuffle = False, num_workers = 1)


    testSize=len(test_dataloader.dataset)
    print("test size: ", testSize)

    testPixelError=0
    headErrorCount=0
    tailErrorCount=0
    model.eval()
    for i, data in enumerate(test_dataloader):
        with torch.set_grad_enabled(False):
            img,label = data[0].to(device), data[1].to(device)
            label_caculate=model(img)
            error,p_error=getDistanceError(label,label_caculate)   # ERROR, ERROR of each point
            testPixelError=testPixelError+error
            if(p_error[0]>3):
                headErrorCount=headErrorCount+1
            if(p_error[1]>5):
                tailErrorCount=tailErrorCount+1


            saveHeatmapResultImg(img,label,savepath,i)

    print("total pixel distance error: ",testPixelError)
    print("average error: ",testPixelError/testSize)
    print("head accuracy: ",(testSize-headErrorCount)/testSize)
    print("tail accuracy: ",(testSize-tailErrorCount)/testSize)

    
    # speed test
    y_size = 320
    x_size = 320
    no_channels = 1
    batch_size = 1
    example_source = torch.rand((batch_size, no_channels, y_size, x_size))

    for j in range(10):
        T1 = time.clock()
        for i in range(1000):
            example_source = example_source.to(device)
            matrix_caculate=model(example_source)
        T2 =time.clock()
        print('模型运行1000次:%s毫秒' % ((T2 - T1)*1000))
        print('平均时间:%s毫秒' % ((T2 - T1)))