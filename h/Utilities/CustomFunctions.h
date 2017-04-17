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

#ifndef __CUSTOM_FUNCTIONS_H
#define __CUSTOM_FUNCTIONS_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#include "HardwareProfile.h"
#include "FatFS/ff.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

    typedef struct {
        uint32_t malloc_count_check;

        union __PACKED {
            uint8_t dmaMatch : 1;
            uint8_t dmaMemToMem : 1;
        } bits;

    } CUSTOM_FUNCTIONS;

    void InitCustomFunctions(void);

    void * custom_malloc2(void **ptr, uint16_t size);
    void * custom_malloc(void *ptr, uint16_t size);
    void custom_free(void **ptr);
    void * custom_memcpy(void * dst, const void * src, size_t size);
    void * custom_memset(void * dst, int value, size_t size);
    int custom_strlen(char *str);
    void * custom_memchr(void * src, int match, size_t size);
    void * custom_memrchr(const void * src, int match, size_t size);

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif