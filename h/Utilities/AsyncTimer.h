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

#ifndef ASYNCTIMER_H
#define ASYNCTIMER_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "HardwareProfile.h"
#include <stdint.h>
#include <stdbool.h>

#define us_SCALE                (GetSystemClock() / 2000000ul)
#define ms_SCALE                (GetSystemClock() / 2000ul)    

#define RTCC_CORE_TIMER
    //#define RTCC_TIMER_2

#if defined(RTCC_CORE_TIMER) && defined(RTCC_TIMER_2)
#error "Only one event at time can be enabled"
#endif

    // This value is used by TCP and other modules to implement timeout actions.
    // For this definition, the Timer must be initialized to use a 1:256 prescalar 
    // in Tick.c.  If using a 32kHz watch crystal as the time base, modify the 
    // Tick.c file to use no prescalar.
#define TICKS_PER_SECOND		((GetPeripheralClock()+128ull)/256ull)	// Internal core clock drives timer with 1:256 prescaler
    //#define TICKS_PER_SECOND		(32768ul)								// 32kHz crystal drives timer with no scalar

    // Represents one second in Ticks
#define TICK_SECOND				((QWORD)TICKS_PER_SECOND)
    // Represents one minute in Ticks
#define TICK_MINUTE				((QWORD)TICKS_PER_SECOND*60ull)
    // Represents one hour in Ticks
#define TICK_HOUR				((QWORD)TICKS_PER_SECOND*3600ull)

    typedef struct __attribute__((__packed__)) {
        // Internal counter to store Ticks.  This variable is incremented in an ISR and 
        // therefore must be marked volatile to prevent the compiler optimizer from 
        // reordering code to use this value in the main context while interrupts are 
        // disabled.
        uint32_t dwInternalTicks;

        // 6-byte value to store Ticks.  Allows for use over longer periods of time.
        uint8_t vTickReading[6] __attribute__((aligned));
    }
    ASYNC_TIMER;

    // Internal calls
    static void GetTickCopy(void);

    // Public calls
    void InitAsyncTimer(void);
    inline void __attribute__((always_inline)) usDelay(uint32_t usDelay);
    inline void __attribute__((always_inline)) msDelay(uint32_t delay_in_ms);

    uint32_t AsyncTickGet(void);
    uint32_t AsyncTickGetDiv256(void);
    uint32_t AsyncTickGetDiv64K(void);
    uint32_t AsyncTickConvertToMilliseconds(uint32_t dwTickValue);
    void AsyncTickUpdate(void);

#ifdef  __cplusplus
}
#endif

#endif  /* ASYNCTIMER_H */
