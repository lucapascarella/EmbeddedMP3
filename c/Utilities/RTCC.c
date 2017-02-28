/*********************************************************************
 *
 *  Real time clock and calendar
 *
 *********************************************************************
 * FileName:        RTCC.c
 * Dependencies:    RTCC.h
 *                  HardwareProfile.h
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
 * This file contains the routines to manage a software RTCC
 *
 * Change History:
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release
 *
 ********************************************************************/


#include "Utilities/RTCC.h"
#include "Utilities/printer.h"

//#include "FatFS/ff.h"

const BYTE dom[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

volatile unsigned long tick; // Used for time benchmarking purposes
volatile BYTE rtccYear, rtccMon, rtccMday; // RTC starting values
volatile BYTE rtccHour, rtccMin, rtccSec;
BYTE holdRtccYear, holdRtccMon, holdRtccMday; // RTC starting values
BYTE holdRtccHour, holdRtccMin, holdRtccSec;

void InitRtcc(void) {

    rtccYear = 2015 - 1980;
    rtccMon = 8;
    rtccMday = 1;

    rtccHour = 12;
    rtccMin = 0;
    rtccSec = 0;

    //// Open 1 ms Timer
    //OpenCoreTimer(CORE_TICK_RATE);
    //// set up the core timer interrupt with a prioirty of 2 and zero sub-priority
    //mConfigIntCoreTimer((CT_INT_ON | CT_INT_PRIOR_1 | CT_INT_SUB_PRIOR_0));


    // Configure Timer 1 using PBCLK as input, 1:256 prescaler
    // Period matches the Timer 1 frequency, so the interrupt handler
    // will trigger at 1000Hz...
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_64, TIMER_2_PERIOD);

    // Set up the timer interrupt with a priority of 2
    INTEnable(INT_T2, INT_ENABLED);
    INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_6);
    INTSetVectorSubPriority(INT_TIMER_2_VECTOR, INT_SUB_PRIORITY_LEVEL_3);
}

//BOOL RtccSetTime(WORD year, BYTE month, BYTE date, BYTE hour, BYTE min, BYTE sec, BYTE day, BOOL forceUpdate) {
//
//}

//void RtccGetTime(int *year, int *month, int *date, int *hour, int *min, int *sec) {
//
//}

//void RtccSetEpochTime(time_t time, BOOL forceUpdate) {
//
//    struct tm *cal;
//
//    cal = gmtime(&time);
//
//    //    RtccSetTime(cal->tm_year + 1900, cal->tm_mon + 1, cal->tm_mday, cal->tm_hour, cal->tm_min, cal->tm_sec, cal->tm_wday, forceUpdate);
//}

//time_tm * rtccGetDateAndTimeTM(void) {
//
//    //int year, month, date, hour, min, sec;
//    static time_tm time;
//
//    RtccHoldTime();
//
//    // Set the time and interval desired
//    time.tm_sec = holdRtccSec;
//    time.tm_min = holdRtccMin;
//    time.tm_hour = holdRtccHour;
//
//    time.tm_mday = holdRtccMday;
//    time.tm_mon = holdRtccMon - 1;
//    time.tm_year = holdRtccYear + 1980 - 1900;
//
//    time.tm_wday = RtccWeekDay(holdRtccYear, holdRtccMon, holdRtccMday);
//
//    //return mktime(&time);
//    return &time;
//}

void rtccGetDate(WORD *rtccYear, BYTE *rtccMon, BYTE *rtccMday) {
    RtccHoldTime();
    *rtccMday = holdRtccMday;
    *rtccMon = holdRtccMon;
    *rtccYear = holdRtccYear + 1980;
}

void rtccGetTime(BYTE *rtccHour, BYTE *rtccMin, BYTE *rtccSec) {
    RtccHoldTime();
    *rtccSec = holdRtccSec;
    *rtccMin = holdRtccMin;
    *rtccHour = holdRtccHour;
}

void rtccGetDateAndTime(WORD *rtccYear, BYTE *rtccMon, BYTE *rtccMday, BYTE *rtccHour, BYTE *rtccMin, BYTE *rtccSec) {
    RtccHoldTime();
    *rtccMday = holdRtccMday;
    *rtccMon = holdRtccMon;
    *rtccYear = holdRtccYear + 1980;
    *rtccSec = holdRtccSec;
    *rtccMin = holdRtccMin;
    *rtccHour = holdRtccHour;
}

void rtccIncDateAndTime(WORD *rtccYear, BYTE *rtccMon, BYTE *rtccMday, BYTE *rtccHour, BYTE *rtccMin, BYTE *rtccSec, WORD secToInc) {

    BYTE n;
    WORD i;

    // Increments a 'fake' RTCC
    for (i = 0; i < secToInc; i++) {
        if (++*rtccSec >= 60) {
            *rtccSec = 0;
            if (++*rtccMin >= 60) {
                *rtccMin = 0;
                if (++*rtccHour >= 24) {
                    *rtccHour = 0;
                    n = dom[*rtccMon - 1];
                    if ((n == 28) && !(*rtccYear & 3))
                        n++;
                    if (++*rtccMday > n) {
                        *rtccMday = 1;
                        if (++*rtccMon > 12) {
                            *rtccMon = 1;
                            *rtccYear++;
                        }
                    }
                }
            }
        }
    }
}

int Rtcc(int argc, char **argv) {

    if (argc < 2) {
        RtccHoldTime();
        printf("%d/%d/%d %02d:%02d:%02d\r\n", holdRtccMday, holdRtccMon, holdRtccYear + 1980, holdRtccHour, holdRtccMin, holdRtccSec);
    } else if (argc == 7) {
        // To do add the set functionality
        holdRtccMday = atoimm(argv[1], 1, 31, 1);
        holdRtccMon = atoimm(argv[2], 1, 12, 1);
        holdRtccYear = atoimm(argv[3], 2010, 2200, 2015) - 1980;

        holdRtccHour = atoimm(argv[4], 0, 23, 1);
        holdRtccMin = atoimm(argv[5], 0, 59, 1);
        holdRtccSec = atoimm(argv[6], 0, 59, 1);

        RtccSetDateAndTime();

    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

/*****************************************************************************
 * Function:	    void CoreTimerHandler(void)
 * PreCondition:
 * Input:           None
 * Output:          None
 * Side Effects:
 * Overview:        FatFs requires a 1ms tick timer to aid
 *		    with low level function timing
 * Note:            Initial Microchip version adapted to work into ISR routine
 *****************************************************************************/
//void __ISR(_CORE_TIMER_VECTOR, IPL2SOFT) CoreTimerHandler(void) {

void __ISR(_TIMER_2_VECTOR, IPL6AUTO) Timer2Handler(void) {

    //static const BYTE dom[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    static int div1k;
    BYTE n;

    // Clear the interrupt flag
    INTClearFlag(INT_T2);

    //// clear the interrupt flag
    //mCTClearIntFlag();
    //// update the period
    //UpdateCoreTimer(CORE_TICK_RATE);

    disk_timerproc(); // call the low level disk IO timer functions
    tick++; // increment the benchmarking timer

    // implement a 'fake' RTCC
    if (++div1k >= 1000) {
        div1k = 0;
        if (++rtccSec >= 60) {
            rtccSec = 0;
            if (++rtccMin >= 60) {
                rtccMin = 0;
                if (++rtccHour >= 24) {
                    rtccHour = 0;
                    n = dom[rtccMon - 1];
                    if ((n == 28) && !(rtccYear & 3))
                        n++;
                    if (++rtccMday > n) {
                        rtccMday = 1;
                        if (++rtccMon > 12) {
                            rtccMon = 1;
                            rtccYear++;
                        }
                    }
                }
            }
        }
    }
}

/*********************************************************************
 * Function:	    DWORD get_fattime(void)
 * PreCondition:
 * Input:           None
 * Output:          Time
 * Side Effects:
 * Overview:        when writing fatfs requires a time stamp
 *		    in this exmaple we are going to use a counter
 *		    If the starter kit has the 32kHz crystal
 *		    installed then the RTCC could be used instead
 * Note:
 ********************************************************************/
void RtccSetDateAndTime(void) {

    INTDisableInterrupts();
    rtccYear = holdRtccYear;
    rtccMon = holdRtccMon;
    rtccMday = holdRtccMday;
    rtccHour = holdRtccHour;
    rtccMin = holdRtccMin;
    rtccSec = holdRtccSec;
    INTEnableInterrupts();
}

/*********************************************************************
 * Function:	    DWORD get_fattime(void)
 * PreCondition:
 * Input:           None
 * Output:          Time
 * Side Effects:
 * Overview:        when writing fatfs requires a time stamp
 *		    in this exmaple we are going to use a counter
 *		    If the starter kit has the 32kHz crystal
 *		    installed then the RTCC could be used instead
 * Note:
 ********************************************************************/
void RtccHoldTime(void) {

    INTDisableInterrupts();
    holdRtccYear = rtccYear;
    holdRtccMon = rtccMon;
    holdRtccMday = rtccMday;
    holdRtccHour = rtccHour;
    holdRtccMin = rtccMin;
    holdRtccSec = rtccSec;
    INTEnableInterrupts();
}

/*********************************************************************
 * Function:	    DWORD get_fattime(void)
 * PreCondition:
 * Input:           None
 * Output:          Time
 * Side Effects:
 * Overview:        when writing fatfs requires a time stamp
 *		    in this exmaple we are going to use a counter
 *		    If the starter kit has the 32kHz crystal
 *		    installed then the RTCC could be used instead
 * Note:
 ********************************************************************/
DWORD get_fattime(void) {
    DWORD tmr;
    //    FILINFO fno;
    FAT_TIME fat;

    RtccHoldTime();

    //    fno.fdate = (WORD) (((holdRtccYear) * 512U) | holdRtccMon * 32U | holdRtccMday);
    //    fno.ftime = (WORD) (holdRtccHour * 2048U | holdRtccMin * 32U | holdRtccSec / 2U);
    //
    fat.fields.day = holdRtccMday;
    fat.fields.month = holdRtccMon;
    fat.fields.year = holdRtccYear;

    fat.fields.sec = holdRtccSec / 2;
    fat.fields.min = holdRtccMin;
    fat.fields.hour = holdRtccHour;
    //    
    //    tmr = (fno.fdate << 16) | fno.ftime;
    //
    //
    //
    ////    tmr = (((DWORD) holdRtccYear) << 25)
    ////            | ((DWORD) holdRtccMon << 21)
    ////            | ((DWORD) holdRtccMday << 16)
    ////            | (WORD) (holdRtccHour << 11)
    ////            | (WORD) (holdRtccMin << 5)
    ////            | (WORD) (holdRtccSec >> 1);
    //
    //    return tmr;
    return fat.val;
}

/*****************************************************************************
 * Function:	    int RtccWeekDay(void)
 * PreCondition:
 * Input:           year, month and day
 * Output:          week day enum
 * Side Effects:
 * Overview:        Calculate day of week in proleptic Gregorian calendar.
 * Note:            Sunday == 0.
 *****************************************************************************/
int RtccWeekDay(int year, int month, int day) {

    int adjustment, mm, yy;

    adjustment = (14 - month) / 12;
    mm = month + 12 * adjustment - 2;
    yy = year - adjustment;

    return (day + (13 * mm - 1) / 5 + yy + yy / 4 - yy / 100 + yy / 400) % 7;
}
