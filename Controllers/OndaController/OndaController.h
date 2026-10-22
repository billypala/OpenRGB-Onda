/*---------------------------------------------------------*\
| OndaController.h                                          |
|                                                           |
|   Driver for Onda (昂达) motherboard RGB controller       |
|                                                           |
|   Protocol reverse engineered from RainAPI.LED.dll        |
|   (native x64, hidapi statically linked).                 |
|   See: Onda-RGB控制器-OpenRGB协议规范.md                   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"
#include "OndaDevices.h"

/*---------------------------------------------------------*\
| Optional init handshake (official software sends an 0x80   |
| feature report and reads back a version at enumeration).   |
| Set to 0 to disable if it disturbs the lighting state.     |
\*---------------------------------------------------------*/
#define ONDA_ENABLE_HANDSHAKE      1

class OndaController
{
public:
    OndaController(hid_device* dev_handle, const char* path, unsigned short vid, unsigned short pid, std::string dev_name);

    std::string     GetLocationString();
    std::string     GetNameString();
    std::string     GetSerialString();
    std::string     GetVersionString();

    unsigned short  GetUSBVID();
    unsigned short  GetUSBPID();

    unsigned short  GetNumBars();
    unsigned short  GetLEDsPerBar();

    bool            Handshake();

    /*-----------------------------------------------------*\
    | Send one bar's colors. last=true marks the final bar   |
    | of a frame (byte 1 = 0xFF, commits the frame).         |
    \*-----------------------------------------------------*/
    void            SetBarLEDs(unsigned char bar, RGBColor* colors, unsigned int num_colors, bool last);

private:
    hid_device*     dev;
    std::string     location;
    std::string     name;
    unsigned short  usb_vid;
    unsigned short  usb_pid;
    unsigned int    firmware_version;
};
