/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        Uart.h
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

#ifndef PRINTER_H
#define	PRINTER_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <string.h>
#include "Compiler.h"

#define PRINTER_BUFFER_SIZE         1024
#define PRI_BUF_ALT_DIM             2u

    typedef struct __attribute__((__packed__)) {
        DWORD alt;
        char txBuf[PRI_BUF_ALT_DIM][PRINTER_BUFFER_SIZE];
    }
    PRINTER_CONFIG;

#ifdef printf
#undef printf
#endif
#define printf(...)         __printf(__VA_ARGS__)
    int __printf(const char * fmt, ...);


#ifdef putc
#undef putc
#endif
#define putc(a)             __putc(a)
    int __putc(char);


#ifdef puts
#undef puts
#endif
#define puts(a)             __puts(a)
    int __puts(const char *p);


#ifdef getc
#undef getc
#endif
#define getc()              __getc();
    char __getc();


#define VER_MIN             1
#define VER_DBG             2
    int verbosePrintf(int level, const char * fmt, ...);


    WORD ConsolWrite(CHAR8 *buffer, WORD count);
    WORD ConsolRead(CHAR8 *buffer, WORD count);

    int Config(int, char **);
    int Verbose(int, char **);

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif // PRINTER_H