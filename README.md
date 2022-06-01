Welcome to Tracking-system-for-larval-zebrafish
======================
Introduction
------------
This tracking system for larval zebrafish is a component of the whole system for whole brain imaging of neural activity in freely behaving larval zebrafish. It is built up in C++.

Pipeline
----------
1. acquire a frame
2. acquire stage position
3. detect the fish, estimate the heading vector of the fish
4. push fish position, fish heading vector and stage position into corresponding queues
5. estimate command that drives the stage using the history of command, stage position, fish position and fish heading vector
6. push command into corresponding queue
7. next loop(it should take 3ms every loop, but actually controlled by the frame rate of camera)

Authors
-------
Tracking-system-for-larval-zebrafish is written by Quan Wen, Fan Gao, Zeguan Wang, Yuming Chai, Daguang Li, Kexin Qi, etc. It is originally a product of the Wen Lab in the Department of Life Science and the HEFEI NATIONAL LABORATORY FOR PHYSICAL SCIENCES AT THE MICROSCALE at University of Science and Technology of China.

Contact
-------
Please contact Daguang Li, by hrcs (at) mail (dot) ustc (dot) edu (dot) cn with questions or feedback.
