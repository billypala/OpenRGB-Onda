/*---------------------------------------------------------*\
| OndaController.cpp                                        |
|                                                           |
|   Driver for Onda (昂达) motherboard RGB controller       |
|                                                           |
|   Protocol (reverse engineered, see protocol spec):       |
|     - Direct mode: one 256-byte feature report per bar     |
|       [0x10+bar, 0x00|0xFF(last), bar, 60*RGB, zero pad]  |
|       with ~1ms between reports                            |
|     - Handshake: send 80 00*7, sleep 20ms, read 8 bytes;   |
|       version = (resp[5] << 8) | resp[6]                   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include <thread>
#include <chrono>
#include "OndaController.h"
#include "StringUtils.h"

using namespace std::chrono_literals;

OndaController::OndaController(hid_device* dev_handle, const char* path, unsigned short vid, unsigned short pid, std::string dev_name)
{
    dev               = dev_handle;
    location          = path;
    name              = dev_name;
    usb_vid           = vid;
    usb_pid           = pid;
    firmware_version  = 0;

    /*-----------------------------------------------------*\
    | NO HANDSHAKE HERE!                                    |
    |                                                            |
    | Verified on hardware: sending GET(0x80) and then data   |
    | SETs on the same handle puts the device into a locked   |
    | state (all SETs fail with ERROR_GEN_FAILURE until the   |
    | device recovers after idle time).                       |
    | Data SETs work fine on a fresh handle WITHOUT any       |
    | handshake, so we simply don't do one.                   |
    |                                                            |
    | (The official software handshakes on a temporary handle |
    |  and closes it before sending data - that pattern also  |
    |  avoids the lock, but no handshake is simpler and       |
    |  proven to work.)                                       |
    \*-----------------------------------------------------*/
}

std::string OndaController::GetLocationString()
{
    return("HID: " + location);
}

std::string OndaController::GetNameString()
{
    return(name);
}

std::string OndaController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

std::string OndaController::GetVersionString()
{
    if(firmware_version == 0)
    {
        return("");
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "%04X", firmware_version);
    return(std::string(buf));
}

unsigned short OndaController::GetUSBVID()
{
    return(usb_vid);
}

unsigned short OndaController::GetUSBPID()
{
    return(usb_pid);
}

unsigned short OndaController::GetNumBars()
{
    return(ONDA_NUM_BARS);
}

unsigned short OndaController::GetLEDsPerBar()
{
    return(ONDA_LEDS_PER_BAR);
}

bool OndaController::Handshake()
{
    unsigned char   out_buf[8];
    unsigned char   in_buf[8];

    memset(out_buf, 0x00, sizeof(out_buf));
    out_buf[0] = ONDA_HANDSHAKE_REPORT_ID;      /* 0x80 */

    if(hid_send_feature_report(dev, out_buf, sizeof(out_buf)) < 0)
    {
        return(false);
    }

    std::this_thread::sleep_for(20ms);

    /*-----------------------------------------------------*\
    | hid_get_feature_report: first byte = report ID        |
    \*-----------------------------------------------------*/
    memset(in_buf, 0x00, sizeof(in_buf));
    in_buf[0] = ONDA_HANDSHAKE_REPORT_ID;

    if(hid_get_feature_report(dev, in_buf, sizeof(in_buf)) < 0)
    {
        return(false);
    }

    firmware_version = (in_buf[5] << 8) | in_buf[6];
    return(true);
}

void OndaController::SetBarLEDs(unsigned char bar, RGBColor* colors, unsigned int num_colors, bool last)
{
    unsigned char report[ONDA_REPORT_LEN];

    if(bar >= ONDA_NUM_BARS)
    {
        return;
    }

    memset(report, 0x00, sizeof(report));

    /*-----------------------------------------------------*\
    | Frame header                                          |
    \*-----------------------------------------------------*/
    report[0] = ONDA_REPORT_ID_BASE + bar;      /* 0x10 + bar */
    report[1] = last ? 0xFF : 0x00;             /* end-of-frame marker */
    report[2] = bar;

    /*-----------------------------------------------------*\
    | RGB payload: 60 LEDs x 3 bytes                        |
    \*-----------------------------------------------------*/
    unsigned int count = num_colors;
    if(count > ONDA_LEDS_PER_BAR)
    {
        count = ONDA_LEDS_PER_BAR;
    }

    for(unsigned int i = 0; i < count; i++)
    {
        report[3 + (i * 3) + 0] = RGBGetRValue(colors[i]);
        report[3 + (i * 3) + 1] = RGBGetGValue(colors[i]);
        report[3 + (i * 3) + 2] = RGBGetBValue(colors[i]);
    }

    hid_send_feature_report(dev, report, sizeof(report));

    /*-----------------------------------------------------*\
    | ~1ms pacing between bar reports (matches official)    |
    \*-----------------------------------------------------*/
    std::this_thread::sleep_for(1ms);
}
