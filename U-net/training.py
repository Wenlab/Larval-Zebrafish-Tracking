import os
import time

import numpy as np
import matplotlib.pyplot as plt
import scipy.ndimage as nd
import math

import torch
import torch.nn as nn
import torch.optim as optim
import torch.utils
import torch.nn.functional as F
from torchsummary import summary

import dataloader as dl

import TrackingKeyPointModel as trackingModel

trainingPath="./dataset/keyPoint220618_320crop/train/"
validationPath="./dataset/keyPoint220618_320crop/val/"
modelSavePath="./model/"
figureSavePath="./figure/"

device=torch.device("cuda:2" if torch.cuda.is_available() else "cpu")


def training(training_params):
    model_name = training_params['model_name']
    num_epochs = training_params['num_epochs']
    batch_size = training_params['batch_size']
    learning_rate = training_params['learning_rate'] 
    initial_path = training_params['initial_path']
    decay_rate = training_params['decay_rate']
    model_save_path = os.path.join(modelSavePath, model_name)

    model=trackingModel.load_network(device,path=initial_path)
    model=model.to(device)
    parameters=model.parameters()
    optimizer=optim.Adam(parameters,learning_rate)
    scheduler = optim.lr_scheduler.LambdaLR(optimizer, lambda epoch: decay_rate**epoch)
    # scheduler = CosineAnnealingWarmRestarts(optimizer,T_0=5,T_mult=2)

    training_loader=dl.TrackingKeyPointDataset(trainingPath)
    validation_loader=dl.TrackingKeyPointDataset(validationPath)
    test_loader=dl.TrackingKeyPointDataset(validationPath)
    training_dataloader=torch.utils.data.DataLoader(training_loader, batch_size = batch_size, shuffle = True, num_workers = 4)#, collate_fn = dl.collate_to_list_Affine)
    validation_dataloader = torch.utils.data.DataLoader(validation_loader, batch_size = batch_size, shuffle = True, num_workers = 4)#, collate_fn = dl.collate_to_list_Affine)
    test_dataloader=torch.utils.data.DataLoader(test_loader, batch_size = 1, shuffle = True, num_workers = 4)

    loss_function=torch.nn.MSELoss()


    # Training starts here
    train_history = []
    val_history = []
    training_size = len(training_dataloader.dataset)
    validation_size = len(validation_dataloader.dataset)
    print("Training size: ", training_size)
    print("Validation size: ", validation_size)

    initial_training_loss = 0.0
    initial_validation_loss = 0.0
    model.eval()
    for i, data in enumerate(training_dataloader):
        with torch.set_grad_enabled(False):
            img,label = data[0].to(device), data[1].to(device)
            # print(img.shape)
            label_caculate=model(img)
            loss=loss_function(label_caculate,label)
            initial_training_loss += loss.item()

    for i, data in enumerate(validation_dataloader):
        with torch.set_grad_enabled(False):
            img,label = data[0].to(device), data[1].to(device)
            label_caculate=model(img)
            loss=loss_function(label_caculate,label)
            initial_validation_loss += loss.item()  
    print("Initial training loss: ", initial_training_loss / (training_size/batch_size))
    print("Initial validation loss: ", initial_validation_loss / (validation_size/batch_size))

    minvalLoss=10000
    for epoch in range(num_epochs):
        bet = time.time()
        print("Current epoch: ", str(epoch + 1) + "/" + str(num_epochs))

        # Training
        train_running_loss = 0.0
        model.train()
        for i, data in enumerate(training_dataloader):
            optimizer.zero_grad()
            with torch.set_grad_enabled(True):
                img,label = data[0].to(device), data[1].to(device)
                label_caculate=model(img)
                loss=loss_function(label_caculate,label)
                # print(loss.item())
                loss.backward()
                train_running_loss += loss.item()
                optimizer.step()
        print("Train Loss: ", train_running_loss / (training_size/batch_size))
        train_history.append(train_running_loss / (training_size/batch_size))


        # Validation
        val_running_loss = 0.0
        model.eval()
        for i, data in enumerate(validation_dataloader):
            with torch.set_grad_enabled(False):
                img,label = data[0].to(device), data[1].to(device)
                label_caculate=model(img)
                loss=loss_function(label_caculate,label)
                val_running_loss += loss.item()
                # print(loss.item())
        print("Val Loss: ", val_running_loss / (validation_size/batch_size))
        print("Now best model Loss: ",minvalLoss/ (validation_size/batch_size))
        val_history.append(val_running_loss / (validation_size/batch_size))

        scheduler.step()

        # save best model
        if val_running_loss<minvalLoss:
            minvalLoss=val_running_loss
            if model_save_path is not None:            
                print("save model...")
                # torch.save(model.state_dict(), model_save_path)
                torch.save(model,model_save_path)

        eet = time.time()
        print("Epoch time: ", eet - bet, "seconds.")
        print("Estimated time to end: ", (eet - bet)*(num_epochs-epoch), "seconds.")
        print("  ")

        plt.figure()
        plt.plot(train_history, "r-")
        plt.plot(val_history, "b-")
        plt.grid(True)
        plt.xlabel("Epoch")
        plt.ylabel("Loss")
        plt.legend(["Train", "Validation"])
        plt.savefig(os.path.join(figureSavePath, model_name + ".png"), bbox_inches = 'tight', pad_inches = 0)
        plt.close()


def run():
    training_params = dict()
    training_params['model_name'] = 'trackKeyPointModel_0618_unet_320crop.pt' # TO DEFINE
    training_params['num_epochs'] = 100
    training_params['batch_size'] = 32
    training_params['learning_rate'] = 0.001
    training_params['initial_path'] = None
    training_params['decay_rate'] = 0.95
    training_params['add_noise'] = True
    training(training_params)   

if __name__ == "__main__":
    run()