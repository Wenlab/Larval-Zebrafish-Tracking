import os

import numpy as np

from PIL import Image
import cv2
import pandas as pd

import torch
import torch.utils as utils

imgSize=320

class TrackingKeyPointDataset(torch.utils.data.Dataset):
    def __init__(self,data_path,transforms=None):
        self.data_path=data_path
        self.img_path=os.path.join(data_path,"img")
        self.label_path=os.path.join(data_path,"label.csv")

        self.all_imgName=os.listdir(self.img_path)
        self.all_imgName.sort()

        self.label=pd.read_csv(self.label_path,header=None)

        self.transforms=transforms

    def __len__(self):
        return len(self.all_imgName)

    def __getitem__(self,idx):
        img_id=self.all_imgName[idx]
        img_path=os.path.join(self.img_path,img_id)

        img=cv2.imread(img_path,cv2.IMREAD_GRAYSCALE)
        # print(img.shape)

        if self.transforms is not None:
            img, _ = self.transforms(img)
        
        label=self.label.values[idx,:]
        label=label*imgSize/320
        label=[[int(label[1]),int(label[0])],
                [int(label[3]),int(label[2])]]    # matlab output x and y coordinate is inverse
        

        # normalize to 0~1
        img=cv2.resize(img,(imgSize,imgSize))
        img=normalize(img)
        heatmap=generateHeatMap(label,imgSize,imgSize,9)


        img_tensor=torch.from_numpy(img.astype(np.float32))
        label_tensor=torch.from_numpy(heatmap.astype(np.float32))

        # if test 2D image
        img_tensor=img_tensor.unsqueeze(dim=0) 

        return img_tensor,label_tensor



def normalize(image):
    if(np.all(image == 0)):
        return image
    else:
        return (image - np.min(image)) / (np.max(image) - np.min(image))

def generateHeatMap(landmark,height,width,sigma):
    heatmaps=[]
    for point in landmark:
        heatmap=np.zeros((height,width))
        heatmap[point[0]][point[1]]=1
        heatmap=cv2.GaussianBlur(heatmap,(sigma,sigma),0)
        am=np.amax(heatmap)
        heatmap /= am / 255
        # heatmap=heatmap/255
        heatmaps.append(heatmap)
        # print(heatmap[149][150])

    heatmaps=np.array(heatmaps)

    # print(heatmaps.shape)
    return heatmaps


if __name__ == '__main__':
    heatmaps=generateHeatMap([[150,150],[200,200]],320,320,9)
    
