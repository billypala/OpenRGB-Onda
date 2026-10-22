/*---------------------------------------------------------*\
| RGBController_Onda.h                                      |
|                                                           |
|   RGBController for Onda (昂达) motherboard RGB           |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "OndaController.h"

class RGBController_Onda : public RGBController
{
public:
    RGBController_Onda(OndaController* controller_ptr);
    ~RGBController_Onda();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    OndaController*   controller;
};
