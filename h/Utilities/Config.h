/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        Config.h
 * Dependencies:    Compiler.h GenericTypeDefs.h HardwareProfile.h
 * Processor:       PIC32MX250F128B
 * Compiler:        Microchip XC32 v1.11a or higher
 * Company:         LP Systems
 * Author:	    Luca Pascarella luca.pascarella@gmail.com
 * Web Site:        www.lucapascarella.it
 *
 * Software License Agreement
 *
 * Copyright (C) 2012-2013 LP Systems  All rights reserved.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 * File Description: Encoder and Decoder state finite machine
 * Change History: In progress
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release (1 September 2013, 16.00)
 *
 ********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#define CONFIG_VERSION          "v0.1"		// Config.h stack version

#include <string.h>
#include <stdlib.h>
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "HardwareProfile.h"
#include "FatFS/ff.h"

#define isUSBEnabled()          (config.gpio[GPIO_6_USB_P].mode == GPIO_S_USB && config.gpio[GPIO_7_USB_N].mode == GPIO_S_USB)



BOOL ConfigInit(void);
//BOOL ConfigScanSection(dictionary *ini);
BOOL ConfigGenerate(void);
BOOL ConfigDumpIni(char *, char *);

typedef struct {

    struct {
        int baudrate;
        BYTE console;
        BYTE echo;
        BYTE port;
        BYTE verbose;
        BYTE versionMajor;
        BYTE versionMinor;
        BYTE bootInfo;
        BYTE log;
    } console;

    struct {
        char* playlist;
        BYTE repeat;
    } play;

    struct {
        char* r_name;
        int prog_over;
        int bitrate;
        int samplerate;
        int bitrate_mode;
        int gain;
        int max_gain;
        int format;
        int adcMode;
    } record;

    union {
        UINT16 Val[4];

        struct __PACKED {
            WORD boost;
            WORD volume;
            CHAR balance;
            CHAR nothing;
        } word;

        struct __PACKED {
            BYTE bass_freq : 4;
            BYTE bass_db : 4;
            BYTE treble_freq : 4;
            CHAR treble_db : 4;
            BYTE right : 8;
            BYTE left : 8;
            CHAR balance : 8;
        } bits;
    }volume;

    struct {
        BYTE mode;
        INT durationInMilliSecs;
        DWORD timeout;
        DWORD durationInTick;

        struct {
            BYTE state : 1; // LSB
            BYTE idle : 1;
            BYTE pull : 2;
            BYTE timeOutEnabled : 1; // MSB
        } bits;
    } gpio[8];

} configuration;

extern configuration config;

//static int handler(void* user, const char* section, const char* name, const char* value);

#ifdef	__cplusplus
}
#endif

#endif