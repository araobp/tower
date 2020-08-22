# AI webcam
 
(work in progress)

## Buffalo webcam

I have got this webcam from Buffalo:

- https://www.buffalo.jp/product/detail/bsw200mbk.html

## Configuration

```
                                OpenCV
                                TensorFlow lite
                                Node.js
[UVC video camera ] --- USB --- [RasPi] --- WiFi --- [HTML5 browser (Chrome on Android or on PC)]

```

## Stadia Methods for positioning a certain object

- Use TensorFlow Lite or OpenCV to detect a spherical object in the image.
- Apply OpenCV's color filters to the image to extract a sphere.
- Measure the size of the sphere.
- Caliculate relative 3D coordinates (x, y, z) from the camera, then add the height (z axis) of the camera.

## Development environments

vi for Node.js, OpenCV and TensorFlow Lite (HTML5, javascript and C/C++ with gcc)

## Code

=> [code](./raspi) ... Work in progress

## References

- [API v2](https://api.ricoh/docs/theta-web-api-v2/)
- [Stadia methods](https://academic.csuohio.edu/duffy_s/Lab_06.pdf)

## Etc

I tested OSC API supported by RICOH Theta, but this project just requires a simple UVC camera with a wider FOV (around 120 degrees).

- [OSC API test program on Android](./android)
