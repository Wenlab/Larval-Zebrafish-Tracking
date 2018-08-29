Pipeline
----------
1. subtract background
2. threshold and morphological operation
3. find contours
4. select the closest contour to the fish centroid in last frame, and set ROI
5. find the centroid by distance transform
6. find the heading direction by fitting line

Notes
----------
1. Initial stage position should be the center to avoid index out of bounds while background sbutraction. Or we can creat a large enough initial global background.
2. To achieve high fps, we can downsample the source images by changing the parameters of the camera and the vi. Elapsed time is about 2~3ms at resolution 520*384.
3. To achieve higher fps, we could record the images rather than avi.
