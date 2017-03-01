/*********************************************************************
 *
 *                  Tick Manager for Timekeeping
 *
 *********************************************************************
 * FileName:        Tick.c
 * Dependencies:    Timer 0 (PIC18) or Timer 1 (PIC24F, PIC24H,
 *					dsPIC30F, dsPIC33F, PIC32)
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.10b or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2010 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
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
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti     6/28/01     Original        (Rev 1.0)
 * Nilesh Rajbharti     2/9/02      Cleanup
 * Nilesh Rajbharti     5/22/02     Rev 2.0 (See version.log for detail)
 * Howard Schlunder		6/13/07		Changed to use timer without
 *									writing for perfect accuracy.
 ********************************************************************/
#define __POWER_C

#include "Utilities/Utilities.h"
#include "Delay/Tick.h"
#include "Delay/Delay.h"
#include "Utilities/printer.h"
#include "BootloaderInfo.h"
#include "FatFS/ff.h"
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
    reboot.Val = FALSE;
    if (mGetPORFlag()) {
        mClearPORFlag();
        reboot.bits.POR = TRUE;
        // execute power on reset handler
        // ...
    } else if (mGetBORFlag()) {
        mClearBORFlag();
        reboot.bits.BOR = TRUE;
        // execute brown out on reset handler
        // ...
    } else if (mGetMCLRFlag()) {
        mClearMCLRFlag();
        reboot.bits.MCLR = TRUE;
        // execute master clear reset handler
        // ...
    } else if (mGetSWRFlag()) {
        mClearSWRFlag();
        reboot.bits.SWR = TRUE;
        // execute software reset handler
        // ...
    } else if (mGetCMRFlag()) {
        mClearCMRFlag();
        reboot.bits.CMR = TRUE;
        // execute configuration mismatch reset handler
        // ...
    } else if (mGetWDTOFlag()) {
        mClearWDTOFlag();
        reboot.bits.WDTO = TRUE;
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
    // The PIC32 Starter Kit debuggers use JTAG and therefore must not
    // disable JTAG.
//    DelayMs(50);
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
        } else {
            LEDs_OFF();
        }
    }
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
//        comma = FALSE;
//        while (argument != NULL) {
//            if (comma) {
//                sprintf(buf, ", %s", argument->argument);
//            } else {
//                if (thisOpt->argNumber == 1)
//                    sprintf(buf, " Argument (1): %s", argument->argument);
//                else
//                    sprintf(buf, " Arguments (%d): %s", thisOpt->argNumber, argument->argument);
//                comma = TRUE;
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