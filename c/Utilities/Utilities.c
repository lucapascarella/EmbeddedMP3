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

#define __UTILITIES_C

#include "Utilities/Utilities.h"
#include "Delay/Tick.h"
#include "Delay/Delay.h"
#include "Utilities/printer.h"
#include "BootloaderInfo.h"
#include "Compiler.h"
#include <p32xxxx.h>


// Global magic variable
_REBOOT reboot;

/****************************************************************************
  Function:
    void InitializeBoard(void)

  Description:
    This routine initializes the hardware.  It is a generic initialization
    routine for many of the Microchip development boards, using definitions
    in HardwareProfile.h to determine specific initialization.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void InitializeSystem(void) {

    // Disable all analog inputs and set these pins as digital inputs
    ANSELA = 0x0000;
    ANSELB = 0x0000;

    // Turn off all LEDs
    LEDs_OFF();

    // Reset all reset indicator
    reboot.Val = false;
    if (mGetPORFlag()) {
        mClearPORFlag();
        reboot.bits.POR = true;
        // execute power on reset handler
        // ...
    } else if (mGetBORFlag()) {
        mClearBORFlag();
        reboot.bits.BOR = true;
        // execute brown out on reset handler
        // ...
    } else if (mGetMCLRFlag()) {
        mClearMCLRFlag();
        reboot.bits.MCLR = true;
        // execute master clear reset handler
        // ...
    } else if (mGetSWRFlag()) {
        mClearSWRFlag();
        reboot.bits.SWR = true;
        // execute software reset handler
        // ...
    } else if (mGetCMRFlag()) {
        mClearCMRFlag();
        reboot.bits.CMR = true;
        // execute configuration mismatch reset handler
        // ...
    } else if (mGetWDTOFlag()) {
        mClearWDTOFlag();
        reboot.bits.WDTO = true;
        // execute watchdog timeout reset handler
        // ...
    }

    // configure for multi-vectored mode
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    // enable interrupts
    INTEnableInterrupts();

    // Enable optimal performance
    SYSTEMConfigPerformance(GetSystemClock());
    // Use 1:1 CPU Core:Peripheral clocks
    OSCSetPBDIV(OSC_PB_DIV_1);

    // Disable JTAG port so we get our I/O pins back, but first
    // wait 50ms so if you want to reprogram the part with
    // JTAG, you'll still have a tiny window before JTAG goes away.
    // DelayMs(50);
    mJTAGPortEnable(DEBUG_JTAGPORT_OFF);

    // Unlock configuration
    CFGCONbits.IOLOCK = 0;

    // Configure SPI Interface remapping the MISO and MOSI signals (SCK is already configured by default)
    SDI1Rbits.SDI1R = 0b0001; // Assigns RPB5 port at SDI1
    RPA4Rbits.RPA4R = 0b0011; // Assigns SDO1 at RPB8 port

    // Configure the UART 2, remapping the U2RX and U2TX signals
    U2RXRbits.U2RXR = 0b0010; // Assigns RPB1 port at U2RX
    RPB0Rbits.RPB0R = 0b0010; // Assigns U2TX at RPB0 port

    // relock configuration
    CFGCONbits.IOLOCK = 1;
}

void PrintRebootStatus(void) {

    extern char *MyScratchPad;

    printf("\r\n");
    if (reboot.bits.POR)
        printf("Power-on Reset has occurred\r\n");
    if (reboot.bits.BOR)
        printf("Brown-out Reset has occurred\r\n");
    if (reboot.bits.MCLR)
        printf("Master Clear (pin) Reset has occurred\r\n");
    if (reboot.bits.SWR)
        printf("Software Reset was executed\r\n");
    if (reboot.bits.CMR)
        printf("Configuration mismatch Reset has occurred\r\n");
    if (reboot.bits.WDTO)
        printf("WDT Time-out has occurred\r\n");

    MyScratchPad[63] = '\0';
    printf("%s\r\n", MyScratchPad);

    /*
    printf("");
    printf("BMXPUPBA 0x%X",BMXPUPBA);
    printf("BMXDKPBA 0x%X",BMXDKPBA);
    printf("BMXDUPBA 0x%X",BMXDUPBA);
    printf("RAM memory size BMXDRMSZ 0x%X (%d KB)",BMXDRMSZ, (BMXDRMSZ>>10));
    printf("FLASH memory size BMXPFMSZ 0x%X (%d KB)",BMXPFMSZ,(BMXPFMSZ>>10));
    printf("BOOT memory size BMXBOOTSZ 0x%X (%d KB)",BMXBOOTSZ,(BMXBOOTSZ>>10));
    printf("");
     */
}

void FlashLight(int speed, int loops, bool reboot) {

    int i;

    for (i = 0; i < loops; i++) {
        DelayMs(speed);
        LED_RED_ON();
        DelayMs(speed);
        LEDs_OFF();
    }

    if (reboot)
        Reset();
    Nop();
}

void Toggle1Second(void) {
    static DWORD t = 0;

    if (TickGet() - t >= TICK_SECOND / 2ul) {
        t = TickGet();
        //LEDs_TOGGLE();
        if (LEDs_TRIS == 1) {
            LED_BLUE_ON();
            printf("Questo e' un test vediamo come va\r\n");
        } else {
            LEDs_OFF();
        }
    }
}

const char * string_rc(FRESULT rc) {

    FRESULT i;
    const char *str =
            "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
            "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
            "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
            "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";

    if (rc != FR_OK) {
        for (i = 0; i != rc && *str; i++) {
            while (*str++);
        }
        return str;
    }
    return NULL;
}

//void printOptionsAndArguments(return_t *rtn) {
//
//    option_t *optList, *thisOpt;
//    argument_t *argList, *thisArg, *argument;
//    char buf[1024];
//    BOOL comma;
//
//    optList = rtn->opt;
//    while (optList != NULL) {
//        thisOpt = optList;
//        optList = optList->next;
//
//        printf("Option: -%c;", thisOpt->option);
//
//        // Prints all arguments
//        argument = thisOpt->nextArgument;
//        comma = false;
//        while (argument != NULL) {
//            if (comma) {
//                sprintf(buf, ", %s", argument->argument);
//            } else {
//                if (thisOpt->argNumber == 1)
//                    sprintf(buf, " Argument (1): %s", argument->argument);
//                else
//                    sprintf(buf, " Arguments (%d): %s", thisOpt->argNumber, argument->argument);
//                comma = true;
//            }
//            printf(buf);
//            argument = argument->nextArgument;
//        }
//    }
//
//    printf("Other: ");
//    argList = rtn->arg;
//    while (argList != NULL) {
//        thisArg = argList;
//        argList = argList->nextArgument;
//        sprintf(buf, "%s ", thisArg->argument);
//        printf(buf);
//    }
//}

static malloc_count_check = 0;

void * custom_malloc2(void **ptr, uint16_t size) {
    if (*ptr != NULL)
        while (1); // Catch here with debugger, this must never happen
    if ((*ptr = malloc(size)) == NULL)
        while (1); // Catch here with debugger, this must never happen
    custom_memset(*ptr, 0x00, size);
    malloc_count_check++;
    return *ptr;
}

void * custom_malloc(void *ptr, uint16_t size) {
    if (ptr != NULL)
        while (1); // Catch here with debugger, this must never happen
    if ((ptr = malloc(size)) == NULL)
        while (1); // Catch here with debugger, this must never happen
    custom_memset(ptr, 0x00, size);
    malloc_count_check++;
    return ptr;
}

void custom_free(void **ptr) {
    if (ptr == NULL)
        while (1); // Catch here with debugger, this must never happen
    if (*ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
        malloc_count_check--;
    }
}

void * custom_memcpy(void * dst, const void * src, size_t size) {

    if (dst != NULL && src != NULL && size != 0x0000) {
        // TODO improve with DMA RAM to RAM fast copy
        memcpy(dst, src, size);

        //while (MEM_TO_MEM_DMA_WORKING());
        //MEM_TO_MEM_DMA_CLR_BTC();
        DmaChnSetTxfer(MEM_TO_MEM_DMA_CHANNEL, src, dst, size, size, size);
        DmaChnStartTxfer(MEM_TO_MEM_DMA_CHANNEL, DMA_WAIT_BLOCK, 0);
        //while (MEM_TO_MEM_DMA_WORKING());
    }
    return dst;
}

void * custom_memset(void * dst, int value, size_t size) {
    // TODO improve with DMA RAM to RAM fast copy
    if (dst != NULL && size != 0x0000) {
        memset(dst, value, size);
        //while (MEM_TO_MEM_DMA_WORKING());
        //MEM_TO_MEM_DMA_CLR_BTC();
        DmaChnSetTxfer(MEM_TO_MEM_DMA_CHANNEL, &value, dst, 1, size, size);
        DmaChnStartTxfer(MEM_TO_MEM_DMA_CHANNEL, DMA_WAIT_BLOCK, 0);
        //while (MEM_TO_MEM_DMA_WORKING());
    }
    return dst;
}

int custom_strlen(char *str) {

    //    int size;
    //    char value;
    if (str != NULL) {
        // TODO improve with DMA match pattern
        // To get the size you have to subtract the address of the src DMA pointer to with base address of "str"
        // Or you can just use DCHxSPTR
        // However a new reopen is required to configure the pattern matching system. After the use you have to restore the previous conditions
        return strlen(str);

        //        size = 65355;
        //        DmaChnSetTxfer(MEM_TO_MEM_DMA_CHANNEL, str, &value, size, size, 1);
        //        DmaChnSetMatchPattern(MEM_TO_MEM_DMA_CHANNEL, '\0');
        //        DmaChnStartTxfer(MEM_TO_MEM_DMA_CHANNEL, DMA_WAIT_BLOCK, 0);
        //        Nop();
    }
    return 0;
}

void * custom_memchr(const void * src, int match, size_t size) {
    // TODO improve with DMA pattern match
    return memchr(src, match, size);
}

void * custom_memrchr(const void * src, int match, size_t size) {

}