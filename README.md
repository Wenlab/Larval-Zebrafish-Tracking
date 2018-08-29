Welcome to Tracking-system-for-larval-zebrafish
======================
Introduction
------------
This tracking system for larval zebrafish is a component of the whole system for whole brain imaging of neural activity in freely behaving larval zebrafish. It is built up by Labview, while the image processing component (fish tracking dll) is written by C++.

Pipeline
------------
1. subtract background
2. threshold and morphological operation
3. find contours
4. select the closest contour to the fish centroid in last frame, and set ROI
5. find the centroid by distance transform
6. find the heading direction by fitting line

Note
------------
1. Initial stage position should be roughly the center to avoid index out of bounds while background sbutraction. Or we can creat a large enough initial global background.
2. To achieve high fps, we can downsample the source images by changing the parameters of the camera and the vi. Elapsed time is about 2~3ms at resolution 520*384.
3. To achieve higher fps, we could record the images rather than avi.

Authors
-------
Tracking-system-for-larval-zebrafish is written by Quan Wen, Fan Gao, Zeguan Wang, Yuming Chai, Daguang Li, etc. It is originally a product of the Wen Lab in the Department of Life Science and the HEFEI NATIONAL LABORATORY FOR PHYSICAL SCIENCES AT THE MICROSCALE at University of Science and Technology of China.

Contact
-------
Please contact Daguang Li, by hrcs (at) mail (dot) ustc (dot) edu (dot) cn with questions or feedback.
