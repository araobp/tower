# RICOH Theta
 
(work in progress)

## Motivation

I want to evaluate Ricoh Theta to see if it can be used as a web cam that streams Motion JPEG (MIME type "multipart/x-mixed-replace").

## Goal

[Goal 1] Develop a simple Android app to control Theta remotely. This is just to evaluate OSC APIs.

[Goal 2] Use Theta as a web cam.

## Theta SC

I have got Ricoh Theta SC.

- https://theta360.com/en/about/theta/sc.html

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
[Ricoh Theta SC] --- USB --- [RasPi] --- WiFi --- [HTML5 browser]
```

I will test the following equirectangular image viewer based on HTML5, in the configuation above: 
https://github.com/akokubo/ThetaViewer

## Code

- [OSC API test program on Android](./android)
- [Web cam](./raspi) ... Work in progress

## References

- [API v2](https://api.ricoh/docs/theta-web-api-v2/)
