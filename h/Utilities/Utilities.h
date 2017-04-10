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

#ifndef __UTILITIES_H
#define __UTILITIES_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#include "HardwareProfile.h"
#include "FatFS/ff.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

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

    typedef union {
        uint8_t Val;

        struct __PACKED {
            uint8_t POR : 1;
            uint8_t BOR : 1;
            uint8_t MCLR : 1;
            uint8_t SWR : 1;
            uint8_t CMR : 8;
            uint8_t WDTO : 1;
        } bits;

    } _REBOOT;

    void InitializeSystem(void);
    void PrintRebootStatus(void);
    void FlashLight(int speed, int loops, bool reboot);
    void Toggle1Second(void);
    const char * string_rc(FRESULT rc);
    //void printOptionsAndArguments(return_t *rtnList);


    void * custom_malloc2(void **ptr, uint16_t size);
    void * custom_malloc(void *ptr, uint16_t size);
    void custom_free(void **ptr);
    void * custom_memcpy(void * dst, const void * src, size_t size);
    void * custom_memset(void * dst, int value, size_t size);
    int custom_strlen(char *str);
    void * custom_memchr(const void * src, int match, size_t size);
    void * custom_memrchr(const void * src, int match, size_t size);

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif