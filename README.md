# Larval-zebrafish-tracking

The larval zebrafish tracking program for ¡°[*All-optical interrogation of brain-wide activity in freely swimming larval zebrafish*](https://www.biorxiv.org/content/10.1101/2023.05.24.542114v1)¡±

#### Introduction

This tracking system for larval zebrafish is a component of the whole system for whole brain imaging of neural activity in freely swimming larval zebrafish. It is built up in C++. The tracking program will detect the fish and control the motion stage to compensate the motion of fish. The tracking system can keep the fish brain complete in the field of view (FoV) of the XLFM in about 60 percent of the time when the fish is swimming. The main program is [CirClassExample.cpp](Track\CirClassExample.cpp). Some codes for estimation of parameters are in [setting-parameters](setting-parameters). Some codes for labelling data to train the U-Net are in [labelling-data-for-DL](labelling-data-for-DL). 

![result](figs/result.png)

#### Pipeline

![pipeline](figs/pipeline.png)


<details>
<summary> Detection of the fish </summary>

We use a U-Net to detect fish in real time. The codes for training and the trained network are in [U-net](U-net). 

</details>

<details>
<summary> Motion stage control </summary>

We adopted the model predictive control (MPC) method in [(1)](https://www.nature.com/articles/nmeth.4429) to control the X-Y motorized stage. We modeled the motion of the stage and the fsh, and then selected the optimal stage input by minimizing future tracking error. This part of code is [MPC_main.cpp](Track\MPC_main.cpp) which will call [MPC.cpp](Track\MPC.cpp).

</details>

#### Contributors
- [Daguang Li](https://github.com/Ali0Li) (`hrcs@mail.ustc.edu.cn`)
- [Kexin Qi](https://github.com/kexin2016) (``)
- [Quan Wen](https://github.com/wenquan) (`qwen@ustc.edu.cn`)
