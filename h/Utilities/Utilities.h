/*********************************************************************
 *
 *  Microchip TCP/IP Stack Include File
 *
 *********************************************************************
 * FileName:        TCPIP.h
 * Dependencies:
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2009 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
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
 *
 * V5.36 ---- STACK_USE_MPFS has been removed.
 ********************************************************************/
#ifndef __UTILITIES_H
#define __UTILITIES_H

#define UTILITIES_VERSION 		"v0.1"		// SIM900 stack version

#include <string.h>
#include <stdlib.h>
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "HardwareProfile.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef INPUT
#define INPUT       1
#endif

#ifndef OUTPUT
#define OUTPUT      0
#endif

#ifndef HIGH
#define HIGH        1
#endif

#ifndef LOW
#define LOW         0
#endif

typedef union{

    UINT8 Val;

    struct __PACKED{
        UINT8 POR:1;
        UINT8 BOR:1;
        UINT8 MCLR:1;
        UINT8 SWR:1;
        UINT8 CMR:8;
        UINT8 WDTO:1;
    } bits;

} _REBOOT;

void InitializeSystem(void);
void PrintRebootStatus(void);
void FlashLight(int speed, int loops, bool reboot);
void Toggle1Second(void);

//void printOptionsAndArguments(return_t *rtnList);

#endif