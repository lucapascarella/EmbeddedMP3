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

#define __CUSTOM_FUNCTIOMS_C

#include <Cpp/c/string.h>

#include "Utilities/CustomFunctions.h"
#include "Utilities/AsyncTimer.h"
#include "Compiler.h"


// Global variables
CUSTOM_FUNCTIONS cf;

void __ISR(_DMA_0_VECTOR + MEM_TO_MEM_DMA_CHANNEL, IPL5AUTO) DmaHandlerMemToMem(void) {
    int evFlags; // event flags when getting the interrupt

    INTClearFlag(INT_SOURCE_DMA(MEM_TO_MEM_DMA_CHANNEL)); // release the interrupt in the INT controller, we're servicing int
    evFlags = DmaChnGetEvFlags(MEM_TO_MEM_DMA_CHANNEL); // get the event flags
    if (evFlags & DMA_EV_BLOCK_DONE) { // just a sanity check. we enabled just the DMA_EV_BLOCK_DONE transfer done interrupt
        cf.bits.dmaMemToMem = true;
        DmaChnClrEvFlags(MEM_TO_MEM_DMA_CHANNEL, DMA_EV_BLOCK_DONE);
    }
}

void __ISR(_DMA_0_VECTOR + MEM_MATCH_DMA_CHANNEL, IPL5AUTO) DmaHandlerMatch(void) {
    int evFlags; // event flags when getting the interrupt

    INTClearFlag(INT_SOURCE_DMA(MEM_MATCH_DMA_CHANNEL)); // release the interrupt in the INT controller, we're servicing int
    evFlags = DmaChnGetEvFlags(MEM_MATCH_DMA_CHANNEL); // get the event flags
    if (evFlags & DMA_EV_BLOCK_DONE) { // just a sanity check. we enabled just the DMA_EV_BLOCK_DONE transfer done interrupt
        cf.bits.dmaMatch = true;
        DmaChnClrEvFlags(MEM_MATCH_DMA_CHANNEL, DMA_EV_BLOCK_DONE);
    }
}

void InitCustomFunctions(void) {

    // Configure the DMA Channel assigned to MEM to MEM fast copy
    DmaChnOpen(MEM_TO_MEM_DMA_CHANNEL, DMA_CHN_PRI2, DMA_OPEN_DEFAULT);
    // set the events: only manual start event
    DmaChnSetEventControl(MEM_TO_MEM_DMA_CHANNEL, DMA_EV_START_IRQ_EN);
    // Reset busy flag indicator for fast MEM to MEM copy
    //MEM_TO_MEM_DMA_SET_BTC();
    cf.bits.dmaMemToMem = true;


    // Configure the DMA Channel assigned to MEM to Match stop
    DmaChnOpen(MEM_MATCH_DMA_CHANNEL, DMA_CHN_PRI2, DMA_OPEN_MATCH);
    // set the events: only manual start event
    DmaChnSetEventControl(MEM_MATCH_DMA_CHANNEL, DMA_EV_MATCH_EN);
    // Reset busy flag indicator for fast MEM to MEM copy
    //MEM_TO_MEM_DMA_SET_BTC();
    // enable the transfer done interrupt: all the characters transferred
    //DmaChnSetEvEnableFlags(MEM_MATCH_DMA_CHANNEL, DMA_EV_BLOCK_DONE);
    // enable system wide multi vectored interrupts
    //INTSetVectorPriority(INT_VECTOR_DMA(MEM_MATCH_DMA_CHANNEL), INT_PRIORITY_LEVEL_5); // set INT controller priority
    //INTSetVectorSubPriority(INT_VECTOR_DMA(MEM_MATCH_DMA_CHANNEL), INT_SUB_PRIORITY_LEVEL_3); // set INT controller sub-priority
    // enable the chn interrupt in the INT controller
    //INTEnable(INT_SOURCE_DMA(MEM_MATCH_DMA_CHANNEL), INT_ENABLED);
    cf.bits.dmaMatch = false;

    cf.malloc_count_check = 0;
}

void * custom_malloc2(void **ptr, uint16_t size) {
    if (*ptr != NULL)
        while (1); // Catch here with debugger, this must never happen
    if ((*ptr = malloc(size)) == NULL)
        while (1); // Catch here with debugger, this must never happen
    custom_memset(*ptr, 0x00, size);
    cf.malloc_count_check++;
    return *ptr;
}

void * custom_malloc(void *ptr, uint16_t size) {
    if (ptr != NULL)
        while (1); // Catch here with debugger, this must never happen
    if ((ptr = malloc(size)) == NULL)
        while (1); // Catch here with debugger, this must never happen
    custom_memset(ptr, 0x00, size);
    cf.malloc_count_check++;
    return ptr;
}

void custom_free(void **ptr) {
    if (ptr == NULL)
        while (1); // Catch here with debugger, this must never happen
    if (*ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
        cf.malloc_count_check--;
    }
}

void * custom_memcpy(void * dst, const void * src, size_t size) {
    if (dst != NULL && src != NULL && size != 0x0000) {
        //while (MEM_TO_MEM_DMA_WORKING());
        //MEM_TO_MEM_DMA_CLR_BTC();
        DmaChnSetTxfer(MEM_TO_MEM_DMA_CHANNEL, src, dst, size, size, size);
        DmaChnStartTxfer(MEM_TO_MEM_DMA_CHANNEL, DMA_WAIT_BLOCK, 0);
        //while (MEM_TO_MEM_DMA_WORKING());
    }
    return dst;
}

void * custom_memset(void * dst, int value, size_t size) {
    if (dst != NULL && size != 0x0000) {
        //while (MEM_TO_MEM_DMA_WORKING());
        //MEM_TO_MEM_DMA_CLR_BTC();
        DmaChnSetTxfer(MEM_TO_MEM_DMA_CHANNEL, &value, dst, 1, size, size);
        DmaChnStartTxfer(MEM_TO_MEM_DMA_CHANNEL, DMA_WAIT_BLOCK, 0);
        //while (MEM_TO_MEM_DMA_WORKING());
    }
    return dst;
}

int custom_strlen(char *str) {

    /*
     * A limitation, the input string must be in RAM space because it will be overwritten by DMA
     */
    int size, len;

    if (str != NULL) {
        size = 0x0000FFFF;
        DmaChnSetTxfer(MEM_MATCH_DMA_CHANNEL, str, str, size, size, 1);
        DmaChnSetMatchPattern(MEM_MATCH_DMA_CHANNEL, '\0');
        do {
            DmaChnStartTxfer(MEM_MATCH_DMA_CHANNEL, DMA_WAIT_BLOCK, 0);
            if (MEM_MATCH_DCHxINTbits.CHERIF == true)
                return strlen(str);
                //return -1;
            len = (DMAADDR - MEM_MATCH_DCHxDSA) & 0x00FFFFFF;
        } while (DMASTATbits.DMACH != MEM_MATCH_DMA_CHANNEL);
        return len;
    }
    return 0;
}

void * custom_memchr(void * src, int match, size_t size) {
    int len;
    uint32_t addr;
    char *s = src;
    /*
     * A limitation, the input string must be in RAM space because it will be overwritten by DMA
     */
    if (src != NULL) {
        DmaChnSetTxfer(MEM_MATCH_DMA_CHANNEL, src, src, size, size, 1);
        DmaChnSetMatchPattern(MEM_MATCH_DMA_CHANNEL, match);
        do {
            DmaChnStartTxfer(MEM_MATCH_DMA_CHANNEL, DMA_WAIT_BLOCK, 0);
            addr = DMAADDR;
            len = (addr - MEM_MATCH_DCHxDSA) & 0x0FFFFFFF;
        } while (DMASTATbits.DMACH != MEM_MATCH_DMA_CHANNEL);
        if (len + 1 == size)
            if (s[len] != match)
                return NULL;
        return (void*) (addr | ((uint32_t) src & 0xF0000000));
    }
    return NULL;
}

void * custom_memrchr(const void * src, int match, size_t size) {

}