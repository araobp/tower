# AI webcam
 
(work in progress)

## Goals

- AI webcam: integration with OpenCV and TensorFlow Lite
- Intrusion detection
- Stadia survey for positioning a person or a certain object
- Auto discovery of web cam's IP address in a LAN segment

## USB video cameras

This project supports any UVC video cameras.

I have got the following cheap cameras supporting UVC:
- https://www.buffalo.jp/product/detail/bsw200mbk.html
- http://www.sac-corp.jp/product/contents/actioncam/ac200.html

## Configuration

```
                                OpenCV
                                TensorFlow lite
                                Node.js
[UVC video camera ] --- USB --> [RasPi] --- WiFi --> [HTML5 browser (Chrome on Android or on PC)]
                                        ---      --> [HTML5 browser (Chrome on Android or on PC)]
                                        ---      --> [HTML5 browser (Chrome on Android or on PC)]
```

## Stadia Methods for positioning a certain object

- Use TensorFlow Lite or OpenCV to detect a spherical object in the image.
- Apply OpenCV's color filters to the image to extract a sphere.
- Measure the size of the sphere.
- Caliculate relative 3D coordinates (x, y, z) from the camera, then add the height (z axis) of the camera.

## Development environments

vi for Node.js, OpenCV and TensorFlow Lite (HTML5, javascript and C/C++ with gcc)

## IP address auto discovery

The web cam advertise IP address and a device ID of its own on UDP port 18084 periodically to the LAN. An Android app receives the IP packet and launch Chrome browser with the following URL format:

```
http://<Advertised IP address>:18082/broadcast/<deviceId>
```

## Code

(Work in progress..)

=> [aicam (C/C++)](./raspi/cpp)

=> [broadcaster.js (node.js)](./raspi/node)

=> [Chrome launcher (Kotlin/Android)](./android)

Sequence diagram
```
  aicam                      broadcaster.js                               Chrome
    |                             |                                         |
    |                             |<--- HTTP GET /broadcast/:deviceid ------|
    |--- HTTP POST image/jpeg --->|                                         |
    |                             |--- 200 OK multipart/x-mixed-replace --->|
    |--- HTTP POST image/jpeg --->|                                         |
    |                             |--- multipart/x-mixed-replace ---------->|
    |--- HTTP POST image/jpeg --->|                                         |
    |                             |--- multipart/x-mixed-replace ---------->|

                    :                                  :
```
    
    



```

## References

- [API v2](https://api.ricoh/docs/theta-web-api-v2/)
- [Stadia methods](https://academic.csuohio.edu/duffy_s/Lab_06.pdf)

## Etc

I tested OSC API supported by RICOH Theta, but this project just requires a simple UVC camera with a wider FOV (around 120 degrees).

- [OSC API test program on Android](./etc)
