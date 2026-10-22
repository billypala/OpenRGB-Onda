/*---------------------------------------------------------*\
| OndaControllerDetect.cpp                                  |
|                                                           |
|   Detector for Onda (昂达) motherboard RGB                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "OndaController.h"
#include "RGBController_Onda.h"

DetectedControllers DetectOndaControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        OndaController*     controller     = new OndaController(dev, info->path, info->vendor_id, info->product_id, name);
        RGBController_Onda* rgb_controller = new RGBController_Onda(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| VID 0x8888 / PID 0x7A95 - confirmed on real hardware       |
| VID 0x8888 / PID 0x7AE0 - variant found in code, untested  |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR("Onda Motherboard RGB (snq.cn Lighting Controller)", DetectOndaControllers, ONDA_VID, ONDA_PID_7A95);
REGISTER_HID_DETECTOR("Onda Motherboard RGB (variant 7AE0)",               DetectOndaControllers, ONDA_VID, ONDA_PID_7AE0);
