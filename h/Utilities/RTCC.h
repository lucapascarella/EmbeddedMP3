/*********************************************************************
 *
 *  Real time clock and calendar
 *
 *********************************************************************
 * FileName:        RTCC.h
 * Dependencies:    HardwareProfile.h
 *                  Compiler.h
 *                  GenericTypeDefs.h
 * Processor:       PIC32MX250F128B
 * Compiler:        Microchip XC32 v1.21 or higher
 * Company:         LP Systems
 * Author:          Luca Pascarella www.lucapascarella.it
 *
 * Software License Agreement
 *
 * Copyright (C) 2012-2013 All rights reserved.
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
 * File Description:
 * This file contains the header of a software RTCC
 *
 * Change History:
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release 3 febbraio 2014, 8.02
 *
 ********************************************************************/

#ifndef RTCC_H
#define	RTCC_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "time.h"

    typedef enum week {
        Sunday = 1, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday
    } week;

    typedef union {
        DWORD val;

        struct __PACKED {
            WORD time;
            WORD data;
        } word;

        struct {
            WORD sec : 5;
            WORD min : 6;
            WORD hour : 5;

            WORD day : 5;
            WORD month : 4;
            WORD year : 7;
        } fields;
    } FAT_TIME;

    typedef struct tm time_tm;

    void InitRtcc(void);
    time_tm * rtccGetDateAndTimeTM(void);

    void rtccGetDate(WORD *rtccYear, BYTE *rtccMon, BYTE *rtccMday);
    void rtccGetTime(BYTE *rtccHour, BYTE *rtccMin, BYTE *rtccSec);
    void rtccGetDateAndTime(WORD *rtccYear, BYTE *rtccMon, BYTE *rtccMday, BYTE *rtccHour, BYTE *rtccMin, BYTE *rtccSec);
    void rtccSetDateAndTime(WORD rtccYear, BYTE rtccMon, BYTE rtccMday, BYTE rtccHour, BYTE rtccMin, BYTE rtccSec);
    void rtccIncDateAndTime(WORD *rtccYear, BYTE *rtccMon, BYTE *rtccMday, BYTE *rtccHour, BYTE *rtccMin, BYTE *rtccSec, WORD secToInc);
    //BOOL RtccSetTime(WORD year, BYTE month, BYTE date, BYTE hour, BYTE min, BYTE sec, BYTE day, BOOL forceUpdate);
    //void RtccGetTime(int *year, int *month, int *date, int *hour, int *min, int *sec);
    //void RtccSetEpochTime(time_t time, BOOL forceUpdate);
    //time_t RtccGetEpochTime(void);
    void RtccSetDateAndTime(void);
    void RtccHoldTime(void);
    int RtccWeekDay(int year, int month, int day);
    int Rtcc(int argc, char **argv);

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif	/* RTCC_H */

