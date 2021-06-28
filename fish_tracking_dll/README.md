Pipeline
----------
1. subtract background
2. threshold and morphological operation
3. find contours
4. select the closest contour to the fish brain centroid in last frame, and set ROI
5. estimate the centerline of fish by fitting line
6. determin the precise brain centroid and heading vector by template matching(using 4 templates around either direction of the centerline)
7. update the background while masking out the fish

Notes
----------
1. Initial stage position should be near the center to avoid index out of bounds while background sbutraction. Or we can creat a large enough initial global background. 
2. Environment: VS2017, OpenCV3.4, 
3. Dependency: 
opencv_aruco340.lib
opencv_bgsegm340.lib
opencv_bioinspired340.lib
opencv_calib3d340.lib
opencv_ccalib340.lib
opencv_core340.lib
opencv_datasets340.lib
opencv_dnn340.lib
opencv_dpm340.lib
opencv_face340.lib
opencv_features2d340.lib
opencv_flann340.lib
opencv_fuzzy340.lib
opencv_hdf340.lib
opencv_highgui340.lib
opencv_imgcodecs340.lib
opencv_imgproc340.lib
opencv_img_hash340.lib
opencv_line_descriptor340.lib
opencv_ml340.lib
opencv_objdetect340.lib
opencv_optflow340.lib
opencv_phase_unwrapping340.lib
opencv_photo340.lib
opencv_plot340.lib
opencv_reg340.lib
opencv_rgbd340.lib
opencv_saliency340.lib
opencv_shape340.lib
opencv_stereo340.lib
opencv_stitching340.lib
opencv_structured_light340.lib
opencv_superres340.lib
opencv_surface_matching340.lib
opencv_text340.lib
opencv_tracking340.lib
opencv_video340.lib
opencv_videoio340.lib
opencv_videostab340.lib
opencv_xfeatures2d340.lib
opencv_ximgproc340.lib
opencv_xobjdetect340.lib
opencv_xphoto340.lib
