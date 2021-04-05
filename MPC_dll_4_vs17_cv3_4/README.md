Pipeline
----------
1. predict fish trajectory assuming that the fish moves along the average heading direction at a uniform speed
2. predict stage positions using a linear time translation invariant model
3. optimize the current command using least squares estimate with L2 penalty to the planned future acceleration
