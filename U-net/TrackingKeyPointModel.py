import numpy as np
import matplotlib.pyplot as plt
import math

import torch
from torch import nn
from torch.nn import functional as F
from torchsummary import summary

import time


class DoubleConv(nn.Module):

    def __init__(self,in_channels,out_channels,channel_reduce=False):
        super(DoubleConv, self).__init__()

        # channel reduction factor
        coefficient = 2 if channel_reduce else 1

        self.down = nn.Sequential(
            nn.Conv2d(in_channels, coefficient * out_channels, kernel_size=(3, 3), padding=1),
            nn.BatchNorm2d(coefficient * out_channels),
            nn.ReLU(inplace=True),
            nn.Conv2d(coefficient * out_channels, out_channels, kernel_size=(3, 3), padding=1),
            nn.BatchNorm2d(out_channels),
            nn.ReLU(inplace=True)
        )
    
    def forward(self, x):
        return self.down(x)


# upsample
class Up(nn.Module):
    def __init__(self, in_channels, out_channels):
        super().__init__()
        # self.up = nn.ConvTranspose2d(in_channels // 2, in_channels // 2, kernel_size=4, stride=2, padding=1)  % if pooling=2
        self.up = nn.ConvTranspose2d(in_channels // 2, in_channels // 2, kernel_size=4, stride=4, padding=0)
        self.conv = DoubleConv(in_channels, out_channels, channel_reduce=True)

    def forward(self, x1, x2):
        x1 = self.up(x1)

        # print(x1.shape, x2.shape)
        x = torch.cat([x1, x2], dim=1)
        x = self.conv(x)
        # print(x.shape)
        return x


# simple U-net
class U_net(nn.Module):

    def __init__(self, device):  
        super(U_net, self).__init__()
        self.device = device
        # downsample
        self.double_conv1 = DoubleConv(1, 32)
        self.double_conv2 = DoubleConv(32, 64)
        self.double_conv3 = DoubleConv(64, 64)

        # upsample
        self.up1 = Up(128, 32)
        self.up2 = Up(64, 16)


        # output
        self.out = nn.Conv2d(16, 2, kernel_size=(1, 1), padding=0)

    def forward(self, x):
        # down
        # print('x.shape: ',x.shape)   # x.shape:  torch.Size([1, 1, 256, 256])
        c1 = self.double_conv1(x)  # c1.shape:  torch.Size([1, 32, 256, 256])
        # print('c1.shape: ',c1.shape)
        p1 = nn.MaxPool2d(4)(c1)  # p1.shape:  torch.Size([1, 32, 128, 128])
        # print('p1.shape: ',p1.shape)
        c2 = self.double_conv2(p1)  # c2.shape:  torch.Size([1, 64, 128, 128])
        # print('c2.shape: ',c2.shape)
        p2 = nn.MaxPool2d(4)(c2)  # p2.shape:  torch.Size([1, 64, 64, 64])
        # print('p2.shape: ',p2.shape)
        c3 = self.double_conv3(p2)  # c3.shape:  torch.Size([1, 128, 64, 64])
        # print('c3.shape: ',c3.shape)

        # up
        u1 = self.up1(c3, c2)  # u1.shape:  torch.Size([1, 128, 32, 32])
        # print('u1.shape: ',u1.shape)
        u2 = self.up2(u1, c1)  # u2.shape:  torch.Size([1, 64, 64, 64])
        # print('u2.shape: ',u2.shape)
        out = self.out(u2)

        

        return out

def load_network(device, path=None):
    model = U_net(device)
    model = model.to(device)
    if path is not None:
        model=torch.load(path)
        model.eval()
    return model


def test_forward_pass():
    device = torch.device("cuda:3" if torch.cuda.is_available() else "cpu")
    model = load_network(device)

    y_size = 320
    x_size = 320
    no_channels = 1

    batch_size = 1
    example_source = torch.rand((batch_size, no_channels, y_size, x_size)).to(device)

    # print(example_source.is_cuda)
    result = model(example_source)
    # print(result)

    for j in range(10):
        T1 = time.clock()
        for i in range(1000):
            example_source = example_source.to(device)
            result=model(example_source)
        T2 =time.clock()
        print('model inference 1000 times:%sms' % ((T2 - T1)*1000))
        print('mean cost time:%sms' % ((T2 - T1)))

def run():
    test_forward_pass()

if __name__ == "__main__":
    run()