/*---------------------------------------------------------*\
| RGBController_Onda.cpp                                    |
|                                                           |
|   RGBController for Onda (昂达) motherboard RGB           |
|                                                           |
|   Layout: 8 zones (LED bars) x 60 LEDs                    |
|   Mode:   Direct (per-LED color)                          |
|                                                           |
|   TODO (see protocol spec section 10):                    |
|     - Built-in effect modes (report [cmd_a0, cmd_a1,      |
|       mode] + payload) are NOT yet implemented - the      |
|       payload format lives in RainAPI.LEDCalc.dll and     |
|       still needs reverse engineering.                    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <algorithm>
#include "RGBController_Onda.h"

/**------------------------------------------------------------------*\
    @name Onda Motherboard RGB
    @category Motherboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectOndaControllers
    @comment
        Reverse engineered from OndaRGB (RainAPI.LED.dll).
        Device: "snq.cn - Lighting Controller" (VID 8888 PID 7A95).
        8 LED bars x 60 LEDs, HID feature report protocol.
\*-------------------------------------------------------------------*/

RGBController_Onda::RGBController_Onda(OndaController* controller_ptr)
{
    controller          = controller_ptr;

    name                = controller->GetNameString();
    vendor              = "Onda";
    description         = "Onda Motherboard RGB Controller (snq.cn Lighting Controller)";
    type                = DEVICE_TYPE_MOTHERBOARD;
    location            = controller->GetLocationString();
    serial              = controller->GetSerialString();

    /*-----------------------------------------------------*\
    | Direct mode only (per-LED color)                      |
    \*-----------------------------------------------------*/
    mode Direct;
    Direct.name         = "Direct";
    Direct.value        = 0xFFFF;
    Direct.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_Onda::~RGBController_Onda()
{
    Shutdown();

    delete controller;
}

void RGBController_Onda::SetupZones()
{
    /*-----------------------------------------------------*\
    | Clear any existing color/LED configuration            |
    \*-----------------------------------------------------*/
    leds.clear();
    colors.clear();
    zones.resize(controller->GetNumBars());

    for(std::size_t bar_idx = 0; bar_idx < zones.size(); bar_idx++)
    {
        zones[bar_idx].name         = "LED Bar " + std::to_string(bar_idx + 1);
        zones[bar_idx].type         = ZONE_TYPE_LINEAR;
        zones[bar_idx].leds_min     = 0;
        zones[bar_idx].leds_max     = controller->GetLEDsPerBar();
        zones[bar_idx].leds_count   = controller->GetLEDsPerBar();
        zones[bar_idx].matrix_map.width  = 0;
        zones[bar_idx].matrix_map.height = 0;
        zones[bar_idx].matrix_map.map.resize(0);

        for(unsigned int led_idx = 0; led_idx < zones[bar_idx].leds_count; led_idx++)
        {
            led new_led;
            new_led.name  = zones[bar_idx].name + ", LED " + std::to_string(led_idx);
            new_led.value = (unsigned int)leds.size();
            leds.push_back(new_led);
        }
    }

    SetupColors();
}

void RGBController_Onda::DeviceUpdateLEDs()
{
    /*-----------------------------------------------------*\
    | Send all 8 bars; the last bar carries the 0xFF        |
    | end-of-frame marker that commits the frame.           |
    \*-----------------------------------------------------*/
    for(std::size_t bar_idx = 0; bar_idx < zones.size(); bar_idx++)
    {
        bool last = (bar_idx == zones.size() - 1);
        controller->SetBarLEDs((unsigned char)bar_idx,
                               zones[bar_idx].colors,
                               zones[bar_idx].leds_count,
                               last);
    }
}

void RGBController_Onda::DeviceUpdateZoneLEDs(int zone)
{
    if((size_t)zone >= zones.size())
    {
        return;
    }

    /*-----------------------------------------------------*\
    | Update only this bar, then re-send the last bar with   |
    | the end-of-frame marker to commit the frame.           |
    \*-----------------------------------------------------*/
    controller->SetBarLEDs((unsigned char)zone,
                           zones[zone].colors,
                           zones[zone].leds_count,
                           false);

    std::size_t last_bar = zones.size() - 1;
    if((size_t)zone != last_bar)
    {
        controller->SetBarLEDs((unsigned char)last_bar,
                               zones[last_bar].colors,
                               zones[last_bar].leds_count,
                               true);
    }
}

void RGBController_Onda::DeviceUpdateSingleLED(int led)
{
    /*-----------------------------------------------------*\
    | Find the zone this LED belongs to and update it       |
    \*-----------------------------------------------------*/
    unsigned int zone_idx = 0;
    unsigned int remaining = (unsigned int)led;

    for(std::size_t bar_idx = 0; bar_idx < zones.size(); bar_idx++)
    {
        if(remaining < zones[bar_idx].leds_count)
        {
            zone_idx = (unsigned int)bar_idx;
            break;
        }
        remaining -= zones[bar_idx].leds_count;
    }

    DeviceUpdateZoneLEDs((int)zone_idx);
}

void RGBController_Onda::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}
