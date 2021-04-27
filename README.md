<h3 align="center">
  <img src="assets/red5pro_logo.png" alt="Red5 Pro Logo" />
</h3>
<p align="center">
  <a href="#about">about</a> &bull;
  <a href="#requirements">requirements</a> &bull;
  <a href="#configuration">configuration</a> &bull;
  <a href="#building">building</a> &bull;
  <a href="#examples">examples</a>
</p>

> **NOTE:** The Red5 Pro Linux SDK is currently in **BETA**.

# Red5 Pro Linux Testbed

This repository contains examples used in testing Red5 Pro Linux SDK. [https://www.red5pro.com](https://www.red5pro.com)

# About

The Red5 Pro Linux SDK supports broadcasting on x86_64 and ARM devices.

This repository demonstrates how to incorporate the Red5 Pro Linux SDK into your projects to start live broadcasts on your Linux desktop and ARM-based devices, such as Raspberry Pi.

> You will need to have a [Red5 Pro Account](https://account.red5pro.com) in order to download the latest Red5 Pro Linux SDK. [Sign Up Today!](https://account.red5pro.com/register)

# Requirements

In order to run the testbed included in this repository you will need a Red5 Pro Server deployed either locally or remotely.

If you already have a [Red5 Pro Account](https://account.red5pro.com), you can find the Red5 Pro Server download at [https://account.red5pro.com/download](https://account.red5pro.com/downloads).

> For more information visit [Red5Pro.com](https://www.red5pro.com).

## Tools

The testbed is built using the `g++` compiler. You can install it by issuing the following:

```sh
sudo apt install build-essential
```

# Building

Included in this repository is a Makefile ([makefile](makfile)) that will compile the source and generate a `testbed.bin` file to be run.

To compile the testbed:

```sh
make clean & make
```

# Configuration

The [tests.json](test.json) configuration file is loaded at runtime by the `testbed.bin` program. This configuration file holds many properties that pertain to the stream for broadcast.

There are several properties you will want to change before and in order to start a broadcast stream on your machine/device.

* [r5sdk_path](https://github.com/infrared5/red5pro-linux-sdk-testbed/blob/master/tests.json#L2) - This is the path to the target SDK (shared object library) for your machine/device. _Use the `amd64` version for Linux machines and `arm` version for ARM devices, such as Raspberry Pi._
* [host](https://github.com/infrared5/red5pro-linux-sdk-testbed/blob/master/tests.json#L4) - This is the host IP address you will be broadcasting to, essentially your local or remote deploy of the [Red5 Pro Server](https://www.red5pro.com).
* [video_device](https://github.com/infrared5/red5pro-linux-sdk-testbed/blob/master/tests.json#L5) - This is the video device number found on your machine/device. [Find out how to find your device](#video-device-access)
* [stream1](https://github.com/infrared5/red5pro-linux-sdk-testbed/blob/master/tests.json#L12) - The name of the stream to broadcast on. It has to be unique - only one stream with any given name can be present on the server.

## Video Device Access

The `video_device` setting in the [tests.json](tests.json) file will be used to access the video device on your machine/device.

> **NOTE** This testbed - using the provided `device.c` from the SDK - only supports the use of a camera with H264 support.

To find the target video device (and number to use for the `video_device` property) utilize the `vfl2` tool:

```sh
v4l2-ctl -d /dev/video0 --list-formats
```

Issuing above to view details about `video0`, will print somethin gof the following:

```sh
  Index       : 0
  Type        : Video Capture
  Pixel Format: 'MJPG' (compressed)
  Name        : Motion-JPEG

  Index       : 1
  Type        : Video Capture
  Pixel Format: 'YUYV'
  Name        : YUYV 4:2:2
```

If you are to issue the command again for `video1`:

```sh
v4l2-ctl -d /dev/video1 --list-formats
```

You will see something similar to:

```sh
 Index       : 0
 Type        : Video Capture
 Pixel Format: 'H264' (compressed)
 Name        : H.264
```

This is the video device you will want to assign by setting the `video_device` configuration property to `1`.

### Video Resolution

To determine which resolution and [width, height and fps properties](https://github.com/infrared5/red5pro-linux-sdk-testbed/blob/master/tests.json#L6-L7), issue the following on the target video device:

```sh
v4l2-ctl -d /dev/video1 --list-formats-ext
```

That will print something similar to:

```sh
Index       : 0
Type        : Video Capture<
Pixel Format: 'H264' (compressed)
Name        : H.264
    Size: Discrete 1920x1080
            Interval: Discrete 0.033s (30.000 fps)
            Interval: Discrete 0.040s (25.000 fps)
            Interval: Discrete 0.067s (15.000 fps)
    Size: Discrete 1280x720
            Interval: Discrete 0.033s (30.000 fps)
            Interval: Discrete 0.040s (25.000 fps)
            Interval: Discrete 0.067s (15.000 fps)
    Size: Discrete 800x600
            Interval: Discrete 0.033s (30.000 fps)
            Interval: Discrete 0.040s (25.000 fps)
            Interval: Discrete 0.067s (15.000 fps)
    Size: Discrete 640x480
            Interval: Discrete 0.033s (30.000 fps)
            Interval: Discrete 0.040s (25.000 fps)
            Interval: Discrete 0.067s (15.000 fps)
    Size: Discrete 640x360
            Interval: Discrete 0.033s (30.000 fps)
            Interval: Discrete 0.040s (25.000 fps)
            Interval: Discrete 0.067s (15.000 fps)
    Size: Discrete 352x288
            Interval: Discrete 0.033s (30.000 fps)
            Interval: Discrete 0.040s (25.000 fps)
            Interval: Discrete 0.067s (15.000 fps)
    Size: Discrete 320x240
            Interval: Discrete 0.033s (30.000 fps)
            Interval: Discrete 0.040s (25.000 fps)
            Interval: Discrete 0.067s (15.000 fps)
    Size: Discrete 1920x1080
            Interval: Discrete 0.033s (30.000 fps)
            Interval: Discrete 0.040s (25.000 fps)
            Interval: Discrete 0.067s (15.000 fps)
```

> To determine which `video_bitrate` to set, please refer to this handy guide: [https://www.red5pro.com/blog/stream-settings-for-live-broadcasts-balancing-latency-and-video-quality/](https://www.red5pro.com/blog/stream-settings-for-live-broadcasts-balancing-latency-and-video-quality/).

# Running

After generating the [compiled testbed](#building) and updating the [configuration settings](#configuration), you can run the testbed by issuing the following:

```sh
./testbed.bin
```

This will start a command line prompt from which you can select which test to run:

```sh
Pulling red5pro from directory: librprosdk/lib/amd64/librprosdk-0.1.0.b7-release.so
Red5Pro SDK version 6.0.0
1) Publish : A basic example of publishing a stream
2) Publish - Shared Object : A publisher with an attatched shared object
Input the number that matches the desired example (0 to exit):
```

# Troubleshooting

## Crash 1

1. If the user app crashes before closing the session, the `Camera` may not be released and the system will require a reboot. Same as other platforms.

# Examples

The following testbed examples are available:

* [Publish](#publish)
* [Publish - Shared Object](#publishsharedobject)

## Publish

The [Publish](tests/Publish/PublishTest.c) test is a basic example of using the configuration settings to start a new broadcast session using the target video device.

## PublishSharedObject

The [PublishSharedObject](tests/PublishSharedObject/PublishSharedObjectTest.c) test builds off the basic Publish test to also  demonstrate how to capture events and metadata messaging from the server, including those related to Shared Objects.
