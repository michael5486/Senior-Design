# Senior-Design

Automated Transport Vehicle

ATV will be able to follow a user through various terrain, over obstacles like curbs and stairs, and keep its cargo area safe and flat. An onboard computer system will be able to identify and follow a designated target within the field of view of the attached Intel Realsense R200 camera.

## Current Progress

Current Work is in Realsense/Projects/UserIdRevised

So far I have accomplished:
* Gathering, processing and outputting joint information
* Gathering, processing, storing location data into location buffer
* Finished initialization phase where ATV learns about user features and joints
* Fixed unreliable world easurements reported from the RealSense SDK
* Implemented Socket-based networking to debug the robot during operation
* Utilized 3 data inputs to figure out which person in FOV is the target user:
** Joints data
*** Head
*** Left Shoulder
*** Right Shoulder
*** Spine Mid
*** Center Mass
** Previous Locations
** userID
* Finished data packaging and serial transmission to Arduino
* When target user leaves FOV, the image is divided into regions and we only look for users to compare against target user in those reasons


