# RICOH Theta
 
(work in progress)

## Motivation

I want to evaluate Ricoh Theta to see if it can be used as a web cam that streams Motion JPEG (MIME type "multipart/x-mixed-replace").

## Goal

[Goal 1] Develop a simple Android app to control Theta remotely. This is just to evaluate OSC APIs.

[Goal 2] Use Theta as a web cam for broadcasting MotionJPEG.

[Goal 3] Employ Stadia Survey method for 3D positioning of a spherical marker object.

## RICOH Theta SC and Buffalo webcam

I have got Ricoh Theta SC.

- https://theta360.com/en/about/theta/sc.html

I will also test this USB web cam from Buffalo:

- https://www.buffalo.jp/product/detail/bsw200mbk.html

## Configuration

[Goal 1]
```
[Ricoh Theta SC] --- WiFi --- [Android]
```

[Goal 2]
```
                            OpenCV
                            TensorFlow lite
                            Node.js
[Ricoh Theta SC] --- USB --- [RasPi] --- WiFi --- [HTML5 browser (Chrome on Android or on PC)]

I will also test an USB webcam of FOV 120 degrees instead of Ricoh Theta SC.
```

I will also test the following equirectangular image viewer based on HTML5, in the configuation above: 
https://github.com/akokubo/ThetaViewer

RasPi works as an image archive server and as a web server based on Node.js. I will also run TensorFlow Lite on RasPi for object detection.

## Stadia Methods for positioning a certain object

- Use TensorFlow Lite or OpenCV to detect a spherical object in the image.
- Apply OpenCV's color filters to the image to extract a sphere.
- Measure the size of the sphere.
- Caliculate relative 3D coordinates (x, y, z) from the camera, then add the height (z axis) of the camera.

## Development environments

- Android Studio (Kotlin)
- vi for Node.js, OpenCV and TensorFlow Lite (HTML5, javascript and C/C++ with gcc)

## Code

- [OSC API test program on Android](./android)
- [Web cam](./raspi) ... Work in progress

## References

- [API v2](https://api.ricoh/docs/theta-web-api-v2/)
- [Stadia methods](https://academic.csuohio.edu/duffy_s/Lab_06.pdf)
