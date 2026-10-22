/*---------------------------------------------------------*\
| OndaDevices.h                                             |
|                                                           |
|   Device definitions for Onda (昂达) motherboard RGB      |
|                                                           |
|   Reverse engineered from OndaRGB / RainAPI.LED.dll       |
|   See: Onda-RGB控制器-OpenRGB协议规范.md                   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

/*---------------------------------------------------------*\
| Onda USB IDs                                              |
|                                                           |
|   Confirmed via registry + code constants:                |
|     mov ecx, 0x8888 / mov edx, 0x7a95                     |
|   Device: "snq.cn - Lighting Controller", UP:FF01         |
\*---------------------------------------------------------*/
#define ONDA_VID                   0x8888
#define ONDA_PID_7A95              0x7A95
#define ONDA_PID_7AE0              0x7AE0

/*---------------------------------------------------------*\
| LED layout                                                |
|                                                           |
|   8 bars (zones) x 60 LEDs = 480 total                    |
|   (devinfo struct: LEDBarCount=8, 60 per bar, 480 total)  |
\*---------------------------------------------------------*/
#define ONDA_NUM_BARS              8
#define ONDA_LEDS_PER_BAR          60

/*---------------------------------------------------------*\
| Protocol constants                                        |
|                                                           |
|   Feature reports via hid_send_feature_report():          |
|     Direct frame (per bar, 256 bytes):                    |
|       [0]      = 0x10 + bar      (report ID)              |
|       [1]      = 0xFF (last bar) / 0x00                   |
|       [2]      = bar index                                |
|       [3..182] = 60 x RGB (3 bytes each)                  |
|       [183..]  = zero padding                             |
|     Handshake: 80 00*7 -> read back 8B                    |
\*---------------------------------------------------------*/
#define ONDA_REPORT_LEN            256
#define ONDA_REPORT_ID_BASE        0x10
#define ONDA_HANDSHAKE_REPORT_ID   0x80

typedef struct
{
    unsigned short              vid;
    unsigned short              pid;
    unsigned short              bars;
    unsigned short              leds_per_bar;
    const char*                 name;
} onda_device;

/*---------------------------------------------------------*\
| Single device entry (both PIDs share the same layout)     |
\*---------------------------------------------------------*/
#define ONDA_NUM_DEVICES           2
