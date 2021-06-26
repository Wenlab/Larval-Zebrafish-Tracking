Introduction
-------------
Model predictive control (MPC) is an advanced method of process control that is used to control a process 
while satisfying a set of constraints. 
Model predictive controllers rely on dynamic models of the process, most often linear empirical models obtained by system identification. 
The main advantage of MPC is the fact that it allows the current timeslot to be optimized, while keeping future timeslots in account. 
This is achieved by optimizing a finite time-horizon, but only implementing the current timeslot and then optimizing again, repeatedly. 
Also MPC has the ability to anticipate future events and can take control actions accordingly. 
PID controllers do not have this predictive ability.

We use nearly the same method in [1] to control the stage movement. 
You can find more details in [1], "Online methods/MPC control of xy motorized stage".

We optimize the prediction of fish motion. While the fish is swiming, it wiggles its tail and head. 
However, the yolk of the fish moves nearly straight forward. 
So we first find the yolk by the head position and heading vector of the fish. 
Then, predict the trajectory of the yolk by simply extrapolating.
Finally, calculate the trajectory of head.

The length of history and future should be determined by the frame rate.

Reference
-------------
1. Kim D H, Kim J, Marques J C, et al. Pan-neuronal calcium imaging with cellular resolution in freely swimming zebrafish[J]. Nature Methods, 2017, 14(11).

Pipeline
----------
1. predict fish trajectory assuming that the fish moves along the average heading direction at a uniform speed
2. predict stage positions using a linear time translation invariant model
3. optimize the current command using least squares estimate with L2 penalty to the planned future command

Notes
----------
1. Environment: VS2017, OpenCV3.4, 
2. Dependency: 
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
