/*
 * Copyright (C) 2017 LP Systems
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 * 
 * Author: Luca Pascarella www.lucapascarella.it
 */

#include "Utilities/AsyncTimer.h"
#include "Compiler.h" 

ASYNC_TIMER at;

/*****************************************************************************
 * Function:        void CoreTimerHandler(void) or void Timer2Handler(void)
 * PreCondition:    The InitRtcc() function must be called before
 * Input:           None
 * Output:          None
 * Overview:        This function will be called every TOGGLES_PER_SEC msec
 *****************************************************************************/
#if defined(RTCC_CORE_TIMER)

void __ISR(_CORE_TIMER_VECTOR, IPL2SOFT) CoreTimerHandler(void) {
    // clear the interrupt flag
    mCTClearIntFlag();
    // update the period
    UpdateCoreTimer(CORE_TICK_RATE);
#elif defined(RTCC_TIMER_2)

void __ISR(_TIMER_2_VECTOR, IPL2AUTO) Timer2Handler(void) {
    // Clear the interrupt flag
    INTClearFlag(INT_T2);
#endif
    // Call the periodic async functions
    // Increment internal high tick counter
    at.dwInternalTicks++;
}

/*****************************************************************************
 * Function:        void InitAsyncTimer(void)
 * PreCondition:    The CoreTimer or the Timer2 must be free to use
 * Input:           None
 * Output:          None
 * Overview:        This is function initialize asynchronous event handler
 *                  with a periodic call of TOGGLES_PER_SEC msec
 *****************************************************************************/
void InitAsyncTimer(void) {

#if defined(RTCC_CORE_TIMER)
    // Open 1 ms Timer
    OpenCoreTimer(CORE_TICK_RATE);
    // set up the core timer interrupt with a priority of 2 and zero sub-priority
    mConfigIntCoreTimer((CT_INT_ON | CT_INT_PRIOR_1 | CT_INT_SUB_PRIOR_0));
    // Set up the timer interrupt with a priority of 2
    INTEnable(INT_CT, INT_ENABLED);
    INTSetVectorPriority(INT_CORE_TIMER_VECTOR, INT_PRIORITY_LEVEL_2);
    INTSetVectorSubPriority(INT_CORE_TIMER_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
#elif defined(RTCC_TIMER_2)
    // Configure Timer 1 using PBCLK as input, 1:256 prescaler
    // Period matches the Timer 1 frequency, so the interrupt handler
    // will trigger at 1000Hz...
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_64, TIMER_2_PERIOD);

    // Set up the timer interrupt with a priority of 2
    INTEnable(INT_T2, INT_ENABLED);
    INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_2);
    INTSetVectorSubPriority(INT_TIMER_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
#endif
    at.dwInternalTicks = 0;
}

inline void __attribute__((always_inline)) usDelay(uint32_t usDelay) {
    register uint32_t startCount = _CP0_GET_COUNT();
    register uint32_t waitCount = usDelay * us_SCALE;
    while (_CP0_GET_COUNT() - startCount < waitCount);
}

inline void __attribute__((always_inline)) msDelay(uint32_t msDelay) {
    register uint32_t startCount = _CP0_GET_COUNT();
    register uint32_t waitCount = msDelay * ms_SCALE;
    while (_CP0_GET_COUNT() - startCount < waitCount);
}


/*****************************************************************************
  Function:
        static void GetTickCopy(void)

  Summary:
        Reads the tick value.

  Description:
        This function performs an interrupt-safe and synchronized read of the
        48-bit Tick value.

  Precondition:
        None

  Parameters:
        None

  Returns:
        None
 ***************************************************************************/
static void GetTickCopy(void) {
    // Perform an Interrupt safe and synchronized read of the 48-bit tick value
  	// PIC32
    do {
        uint32_t dwTempTicks;
        WORD_VAL wTemp;

        IEC0SET = _IEC0_T1IE_MASK; // Enable interrupt
        Nop();
        IEC0CLR = _IEC0_T1IE_MASK; // Disable interrupt

        // Get low 2 bytes
        wTemp.Val = TMR1;
        at.vTickReading[0] = wTemp.v[0];
        at.vTickReading[1] = wTemp.v[1];
        //((volatile WORD*)vTickReading)[0] = TMR1;

        // Correct corner case where interrupt increments byte[4+] but
        // TMR1 hasn't rolled over to 0x0000 yet
        dwTempTicks = at.dwInternalTicks;

        // PIC32MX3XX/4XX devices trigger the timer interrupt when TMR1 == PR1
        // (TMR1 prescalar is 0x00), requiring us to undo the ISR's increment
        // of the upper 32 bits of our 48 bit timer in the special case when
        // TMR1 == PR1 == 0xFFFF.  For other PIC32 families, the ISR is
        // triggered when TMR1 increments from PR1 to 0x0000, making no special
        // corner case.
#if __PIC32_FEATURE_SET__ <= 460
        //if(((WORD*)vTickReading)[0] == 0xFFFFu)
//        if (wTemp.Val == 0xFFFFu)
//            dwTempTicks--;
#elif !defined(__PIC32_FEATURE_SET__)
#error __PIC32_FEATURE_SET__ macro must be defined.  You need to download a newer C32 compiler version.
#endif

        // Get high 4 bytes
        at.vTickReading[2] = ((BYTE*) & dwTempTicks)[0];
        at.vTickReading[3] = ((BYTE*) & dwTempTicks)[1];
        at.vTickReading[4] = ((BYTE*) & dwTempTicks)[2];
        at.vTickReading[5] = ((BYTE*) & dwTempTicks)[3];
    } while (IFS0bits.T1IF);
    IEC0SET = _IEC0_T1IE_MASK; // Enable interrupt
}

/*****************************************************************************
  Function:
        uint32_t TickGet(void)

  Summary:
        Obtains the current Tick value.

  Description:
        This function retrieves the current Tick value, allowing timing and
        measurement code to be written in a non-blocking fashion.  This function
        retrieves the least significant 32 bits of the internal tick counter,
        and is useful for measuring time increments ranging from a few
        microseconds to a few hours.  Use TickGetDiv256 or TickGetDiv64K for
        longer periods of time.

  Precondition:
        None

  Parameters:
        None

  Returns:
        Lower 32 bits of the current Tick value.
 ***************************************************************************/
uint32_t AsyncTickGet(void) {
    uint32_t dw;

    GetTickCopy();
    ((BYTE*) & dw)[0] = at.vTickReading[0]; // Note: This copy must be done one
    ((BYTE*) & dw)[1] = at.vTickReading[1]; // byte at a time to prevent misaligned
    ((BYTE*) & dw)[2] = at.vTickReading[2]; // memory reads, which will reset the PIC.
    ((BYTE*) & dw)[3] = at.vTickReading[3];
    return dw;
}

/*****************************************************************************
  Function:
    uint32_t TickGetDiv256(void)

  Summary:
    Obtains the current Tick value divided by 256.

  Description:
    This function retrieves the current Tick value, allowing timing and
    measurement code to be written in a non-blocking fashion.  This function
    retrieves the middle 32 bits of the internal tick counter,
    and is useful for measuring time increments ranging from a few
    minutes to a few weeks.  Use TickGet for shorter periods or TickGetDiv64K
    for longer ones.

  Precondition:
    None

  Parameters:
    None

  Returns:
    Middle 32 bits of the current Tick value.
 ***************************************************************************/
uint32_t AsyncTickGetDiv256(void) {
    uint32_t dw;

    GetTickCopy();
    ((BYTE*) & dw)[0] = at.vTickReading[1]; // Note: This copy must be done one
    ((BYTE*) & dw)[1] = at.vTickReading[2]; // byte at a time to prevent misaligned
    ((BYTE*) & dw)[2] = at.vTickReading[3]; // memory reads, which will reset the PIC.
    ((BYTE*) & dw)[3] = at.vTickReading[4];

    return dw;
}

/*****************************************************************************
  Function:
    uint32_t TickGetDiv64K(void)

  Summary:
    Obtains the current Tick value divided by 64K.

  Description:
    This function retrieves the current Tick value, allowing timing and
    measurement code to be written in a non-blocking fashion.  This function
    retrieves the most significant 32 bits of the internal tick counter,
    and is useful for measuring time increments ranging from a few
    days to a few years, or for absolute time measurements.  Use TickGet or
    TickGetDiv256 for shorter periods of time.

  Precondition:
    None

  Parameters:
    None

  Returns:
    Upper 32 bits of the current Tick value.
 ***************************************************************************/
uint32_t AsyncTickGetDiv64K(void) {
    uint32_t dw;

    GetTickCopy();
    ((BYTE*) & dw)[0] = at.vTickReading[2]; // Note: This copy must be done one
    ((BYTE*) & dw)[1] = at.vTickReading[3]; // byte at a time to prevent misaligned
    ((BYTE*) & dw)[2] = at.vTickReading[4]; // memory reads, which will reset the PIC.
    ((BYTE*) & dw)[3] = at.vTickReading[5];

    return dw;
}

/*****************************************************************************
  Function:
    uint32_t TickConvertToMilliseconds(uint32_t dwTickValue)

  Summary:
    Converts a Tick value or difference to milliseconds.

  Description:
    This function converts a Tick value or difference to milliseconds.  For
    example, TickConvertToMilliseconds(32768) returns 1000 when a 32.768kHz
    clock with no prescaler drives the Tick module interrupt.

  Precondition:
    None

  Parameters:
    dwTickValue	- Value to convert to milliseconds

  Returns:
    Input value expressed in milliseconds.

  Remarks:
    This function performs division on DWORDs, which is slow.  Avoid using
    it unless you absolutely must (such as displaying data to a user).  For
    timeout comparisons, compare the current value to a multiple or fraction
    of TICK_SECOND, which will be calculated only once at compile time.
 ***************************************************************************/
uint32_t TickConvertToMilliseconds(uint32_t dwTickValue) {
    return (dwTickValue + (TICKS_PER_SECOND / 2000ul)) / ((uint32_t) (TICKS_PER_SECOND / 1000ul));
}


