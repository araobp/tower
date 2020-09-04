# AI Webcam
 
 <img src="./doc/uvc_camera.png" width=200px>
 
## Features

- AI webcam: integration with OpenCV and TensorFlow Lite
- Intrusion detection
- Auto discovery of web cam's IP address in a LAN segment
- MotionJPEG broadcasting server
- Optional features: low-power wireless, temperature/humidity sensor etc..

## USB video cameras

This project supports any UVC video cameras.

I have got the following cheap cameras supporting UVC:
- https://www.buffalo.jp/product/detail/bsw200mbk.html (FOV 120 degrees horizontal)
- http://www.sac-corp.jp/product/contents/actioncam/ac200.html (FOV 78 degrees horizontal/ 46 degrees vertical)

## Configuration

```
                                OpenCV
                                TensorFlow lite
                                Node.js
[UVC video camera ] --- USB --> [RasPi] --- WiFi --> [HTML5 browser (Chrome on Android or on PC)]
                                        ---      --> [HTML5 browser (Chrome on Android or on PC)]
                                        ---      --> [HTML5 browser (Chrome on Android or on PC)]
```

## Development environments

vi for Node.js, OpenCV and TensorFlow Lite (HTML5, javascript and C/C++ with gcc)

## IP address auto discovery

The web cam advertise IP address and a device ID of its own on UDP port 18084 periodically to the LAN. An Android app receives the IP packet and launch Chrome browser with the following URL format: http://<Advertised IP address>:18082/broadcast/<deviceId>

```

  Web cam          Launcher app on Android
     |                      |
     |----- UDP 18084 ----->|
     |                      |
     |----- UDP 18084 ----->|
     |                      |
     |----- UDP 18084 ----->| http://<Advertised IP address>:18082/broadcast/<deviceId>
     |                      |
                            |
                            |
 Motion JPEG broadcaster    |
     |                      |
     |<---- HTTP GET -------|
     |---- Motion JPEG ---->|
     |---- Motion JPEG ---->|
     |---- Motion JPEG ---->|
     |         :            |
     
```

## Code

=> [aicam (C/C++)](./raspi/cpp)

=> [broadcaster.js (node.js)](./raspi/node)

=> [Chrome launcher (Kotlin/Android)](./android)

Sequence diagram
```
  aicam                      broadcaster.js                        Chrome launcher app
    |                             |                                         |
    |--- Advertisment UDP packet ------------------------------------------>| -> URL of AI Webcam
    |              :              |                                         |          |
                                                                                       |
  aicam                      broadcaster.js                              Chrome <------+
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

## Etc

I tested OSC API supported by RICOH Theta, but this project just requires a simple UVC camera with a wider FOV.

- [OSC API test program on Android](./etc)
