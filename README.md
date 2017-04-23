# Senior-Design

Automated Transport Vehicle

ATV will be able to follow a user through various terrain, over obstacles like curbs and stairs, and keep its cargo area safe and flat. An onboard computer will identify and follow a designated target within the field of view of the attached Intel Realsense R200 camera.

## What's in this repository?

The code that operates the RealSense R200 and the Arduino Mega are in this repository. All Realsense code is located in `/Realsense` while Arduino code is located in `/Motors`. The final RealSense project does have some Arduino code mixed in to enable it to communicate over the Serial connection.

## Current Progress

A lot of time went in to getting the PersonTracking module into working condition. We inherited a mostly functional code-base from Intel, but very little correct documentation regarding the internals. Through a large amount of trial and error, we got this module into working condition and have iteratively added features. 

So far we have accomplished:
* Gathering, processing and outputting joint information
* Gathering, processing, storing location data into location buffer
* Finished initialization phase where ATV learns about target user features and joints
* Fixed unreliable world measurements reported from the RealSense SDK
* Implemented Socket-based networking to debug the robot during operation
* Utilized 3 data inputs to figure out which person in FOV is the target user:
	* Joint data
	* Previous Locations
	* userID
* Finished data packaging and serial transmission to Arduino
* When target user leaves FOV, the image is divided into regions. ATV only attempts to find TU in the last-known region


## Joint Data

In case anyone would like to continue development using the RealSense person tracking module, check out our code. Despite what the SDK documentation says, only a few joints were confirmed working. These joints are enumerated in the table below. We only use the head, shoulders, spine and center of mass in calculations. 

Realsense Joint |  Our Code
----------------|----------
JOINT_HEAD | JOINT_HEAD
JOINT_LEFT_SHOULDER | JOINT_SHOULDER_LEFT
JOINT_RIGHT_SHOULDER | JOINT_SHOULDER_RIGHT
JOINT_SPINE_MID | JOINT_SPINE_MID
from personModule | JOINT_CENTER_MASS


## Combining Visual C++, Arduino and DC motors

`RealSense/userIDMotorsSimple`

Code for controlling motors based on what the RealSense R200 sees. This is the current code operating on the ATV. First, the initialization phase occurs. 

```c++
while (!isInitialized) {
	/* Waits until new frame is available and locks it for application processing */
	sts = pp->AcquireFrame(false);

	/* Render streams */
	PXCCapture::Sample *sample = pp->QuerySample();

	if (sample) {
		PXCPersonTrackingModule* personModule = pp->QueryPersonTracking();

		/* If no persons are visible, renders and releases current frame */
		if (personModule == NULL) {
			if (sample->depth && !renderd.RenderFrame(sample->depth)) break;
			if (sample->color && !renderc.RenderFrame(sample->color)) break;
			pp->ReleaseFrame();
			continue;
		}

		int numPeople = personModule->QueryOutput()->QueryNumberOfPeople();

		/* Found a person */
		if (numPeople == 1) {
			/* When this method is called enough times, it will return true and break initialization loop */
			if (initializeTargetUser(personModule)) {
				break;
			}

		}

		/* Releases lock so pipeline can process next frame */
		pp->ReleaseFrame();

	}
}

printf("Target initialized: \n");
printf("torsoHeight = %f\n", targetUserTorsoHeight);
printf("shoulderWidth = %f\n", targetUserShoulderWidth);
```

Brief video of the initialization phase occurring:

[![IMAGE ALT TEXT HERE](http://img.youtube.com/vi/NyF7QaYOzA4/0.jpg)](http://www.youtube.com/watch?v=NyF7QaYOzA4)


After this initialization phase, the main operation loop occurs. In this loop, the ATV is constantly locating the target user. When the ATV successfully locates the target user, it calculates the corresponding motor controls and relays that to the Arduino Mega for parsing. In our initial control scheme, these controls are simplified into going forwards, going backwards, turning left and turning right. These commands are packages into 1-byte commands and sent Serially. With a size of 1-byte, in the event of a temporary data transmission function the ATV will continue to run unhindered. The ATV will just miss a control signal, but continue correctly upon seing successive command signals. 


`Motors/readSerialMotorTest/`

Arduino is serially connected to the Windows computer, and instructions are sent in 1-byte increments. This Arduino sketch parses motor control signals sent from the ATV's onboard computer. These signals are parsed and the proper Pulse Width Modulation (PWM) signals are outputted. A series of H bridges enables the DC motors to both go forwards and backwards, giving the ATV a very small turning radius.

## Special Cases

The ATV has en easy time following the target user if this user always stays within its field of view

## To run the code

1. Ensure the RealSense is plugged into the Windows computer. 
2. Ensure the Arduino is also plugged into the Windows computer. 
3. Compile and run sketch on the Arduino. 
4. Compile and run the code on the Windows computer. 
5. Wait for initialization to finish.
6. ???
7. Profit



