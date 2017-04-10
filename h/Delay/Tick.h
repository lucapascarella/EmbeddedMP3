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

#ifndef __TICK_H
#define __TICK_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#include "Compiler.h"
#include "HardwareProfile.h"

// All TICKS are stored as 32-bit unsigned integers.
// This is deprecated since it conflicts with other TICK definitions used in 
// other Microchip software libraries and therefore poses a merge and maintence 
// problem.  Instead of using the TICK data type, just use the base DWORD data 
// type instead.
typedef __attribute__((__deprecated__)) DWORD TICK;

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


void TickInit(void);
DWORD TickGet(void);
DWORD TickGetDiv256(void);
DWORD TickGetDiv64K(void);
DWORD TickConvertToMilliseconds(DWORD dwTickValue);
void TickUpdate(void);


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif
