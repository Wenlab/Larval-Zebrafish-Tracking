Pipeline
----------
1. subtract background
2. threshold and morphological operation
3. find contours
4. select the closest contour to the fish centroid in last frame, and set ROI
5. find the heading direction by fitting line and the relationship between barycenter and center of the fish
6. determin the precise centroid and heading vector by template matching

Notes
----------
1. Initial stage position should be the center to avoid index out of bounds while background sbutraction. Or we can creat a large enough initial global background.
