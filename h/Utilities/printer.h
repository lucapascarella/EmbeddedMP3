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

#ifndef PRINTER_H
#define	PRINTER_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "Compiler.h"

#define PRINTER_BUFFER_SIZE         1024
#define PRI_BUF_ALT_DIM             2u

    typedef struct __attribute__((__packed__)) {
        uint32_t alt;
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


#define VER_NONE            0
#define VER_MIN             1
#define VER_ERR             2
#define VER_DBG             3
    int verbosePrintf(int level, const char * fmt, ...);

    int consolePrint(uint8_t *buffer, uint16_t count);
    
    uint16_t consoleWrite(uint8_t *buffer, uint16_t count);
    uint16_t consoleRead(uint8_t *buffer, uint16_t count);

    void InitPinter(void);
    
    int Config(int, char **);
    int Verbose(int, char **);

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif // PRINTER_H