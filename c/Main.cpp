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


// PIC32MX270F256B Configuration Bit Settings

// 'C' source line config statements

// DEVCFG3
// USERID = No Setting
#pragma config PMDL1WAY = OFF           // Peripheral Module Disable Configuration (Allow multiple reconfigurations)
#pragma config IOL1WAY = OFF            // Peripheral Pin Select Configuration (Allow multiple reconfigurations)
#pragma config FUSBIDIO = OFF           // USB USID Selection (Controlled by Port Function)
#pragma config FVBUSONIO = OFF          // USB VBUS ON Selection (Controlled by Port Function)

// DEVCFG2
#pragma config FPLLIDIV = DIV_4         // PLL Input Divider (4x Divider)
#pragma config FPLLMUL = MUL_20         // PLL Multiplier (20x Multiplier)
#pragma config UPLLIDIV = DIV_5         // USB PLL Input Divider (5x Divider)
#pragma config UPLLEN = ON              // USB PLL Enable (Enabled)
#pragma config FPLLODIV = DIV_2         // System PLL Output Clock Divider (PLL Divide by 2)

// DEVCFG1
#pragma config FNOSC = PRIPLL           // Oscillator Selection Bits (Primary Osc w/PLL (XT+,HS+,EC+PLL))
#pragma config FSOSCEN = OFF            // Secondary Oscillator Enable (Disabled)
#pragma config IESO = OFF               // Internal/External Switch Over (Disabled)
#pragma config POSCMOD = HS             // Primary Oscillator Configuration (HS osc mode)
#pragma config OSCIOFNC = OFF           // CLKO Output Signal Active on the OSCO Pin (Disabled)
#pragma config FPBDIV = DIV_1           // Peripheral Clock Divisor (Pb_Clk is Sys_Clk/1)
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor Selection (Clock Switch Disable, FSCM Disabled)
#pragma config WDTPS = PS32768		// Watchdog Timer Postscaler (1:32768)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable (Watchdog Timer is in Non-Window Mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (WDT Disabled (SWDTEN Bit Controls))
#pragma config FWDTWINSZ = WINSZ_25     // Watchdog Timer Window Size (Window Size is 25%)

// DEVCFG0
#pragma config JTAGEN = OFF             // JTAG Enable (JTAG Disabled)
#pragma config ICESEL = ICS_PGx3        // ICE/ICD Comm Channel Select (Communicate on PGEC3/PGED3)
#pragma config PWP = OFF                // Program Flash Write Protect (Disable)
#pragma config BWP = OFF                // Boot Flash Write Protect bit (Protection Disabled)
#pragma config CP = OFF                 // Code Protect (Protection Disabled)


/*
 * Header section declaration in accord to C++ standard
 */
extern "C" {
#include "Delay/Tick.h"
#include "USB/usb.h"
#include "HardwareProfile.h"
#include "usb_config.h"
#include <p32xxxx.h>
#include <stdio.h>
#include "USB/usb_function_msd.h"
#include "USB/usb_function_cdc.h"
#include "Utilities/USB.h"
#include "Main.hpp"
#include "Utilities/Utilities.h"
#include "Utilities/Logging.h"
#include "Utilities/Config.h"
#include "Utilities/Uart.h"
#include "Utilities/printer.h"
#include "Utilities/CustomFunctions.h"
#include "Delay/Delay.h"
#include "MP3/MP3.h"
#include "MP3/Play.h"
#include "MP3/Record.h"
#include "Utilities/GPIO.h"
#include "FatFS/ff.h"
#include "FatFS/diskio.h"
#include "CommandLineInterpreter.h"
#include "SingleCharacterCommands.h"
#include "I2CSlave.h"
#include "Utilities/ADC.h"
#include "Utilities/RTCC.h"
#include "Utilities/ADC.h"
#include "Utilities/Optlist.h"
#include "MP3/VS1063.h"
#include "Commands.h"
}

#include "CLI.hpp"
#include "MP3/CCPlay.h"
#include "Commands/Playback.hpp"
#include "Test.hpp"
#include "Commands/CommandsList.hpp"
#include <cstdlib>

using namespace std;

// MIPS C32 Exception Handlers
// If your code gets here, you either tried to read or write a NULL pointer,
// or your application overflowed the stack by having too many local variables or parameters declared.
HardwareException he;
static unsigned int address;

void __attribute__((nomips16)) _general_exception_handler(unsigned cause, unsigned status) {

    asm volatile("mfc0 %0,$13" : "=r" (he));
    asm volatile("mfc0 %0,$14" : "=r" (address));

    Nop();
    FlashLight(50, 50, TRUE);
    Nop();

    // To return from the exception, the EPC register must be incremented by 4 to
    // point to the next instruction.  Since the EPC register cannot be manipulated
    // directly, the following code peforms the addition
    Nop(); // trap here for debug
    asm("mfc0 $v0, $14"); // $v0 = EPC (=> $14 means EPC)
    asm("addiu $v0, $v0, 4"); // $v0 += 4
    asm("mtc0 $v0, $14"); // EPC = $v0
}



/** PRIVATE PROTOTYPES *********************************************/
void USBDeviceTasks(void);

// FAT File System global structure
FATFS Fatfs;
// Global structures to manipulate files, candidate to removal
FILINFO finfo;
DIR dir;
FIL fstream2, ftmp1, ftmp2;
// Scratch pad used to share information with bootloader
char * MyScratchPad = (char *) (0xA0000000 + (0x10000 - 0x0040));

/*
 * Main entry application
 */
int main(int argc, char** argv) {

    CCPlay *ccp;
    CLI *cli;
    CommandsList *cmds;

    //    CommandBase *pb;
    //    pb = new Playback();
    //    pb->getStatistics();
    //    pb->playback(argc, argv);
    
    
//    int a;
//    Test *t = new Test(1, 2);
//    a = t->testIt();
//    t->callException(12);
    //
    //    Test tt(3, 4);
    //    a = tt.testIt();

    int play = FALSE, rec = FALSE;
    BOOL logResults;

    // Initialize Watch Dog Timer
    EnableWDT();

    // Initialize application specific hardware
    InitializeSystem();
    
    // Initialize custom functions
    InitCustomFunctions();

    // Initialize Disk IO
    if (disk_initialize(0) != RES_OK)
        FlashLight(250, 50, true);

    // Initialize (Mount) File System structure
    if (f_mount(&Fatfs, "0", 1) != FR_OK)
        FlashLight(250, 50, true);

    // Initialize virtual Real Time Clock Calendar
    InitRtcc();

    // Open the LOG file and append to it the new stream
    logResults = openLogFile();
    if (logResults == false)
        FlashLight(350, 50, false);

    // Initialize the INI file
    if (ConfigInit() == false)
        FlashLight(150, 50, true);


    // Initializes USB module SFRs and firmware variables to known state
    if (isUSBEnabled())
        USBDeviceInit(); //usb_device.c.

    // Initialize UART
    if (isUARTEnabled())
        UartInit();

    // Initialize I2C bus if gpio are correctly selected
    if (isI2CEnabled())
        InitI2C();

    // Initialize ADC if gpio are correctly selected
    if (isADCEnabled())
        InitADC();


    // Print extended boot info and status
    if (config.console.bootInfo == 2)
        PrintRebootStatus();

    // Print welcome message and version
    if (config.console.bootInfo >= 1)
        printf("Embedded MP3 Player v%d.%d\r\n", config.console.versionMajor, config.console.versionMinor);


    // Initialize the accurate time-keeping based on Timer 1
    // The Tick module exists to assist with the implementation of non-blocking delays and timeouts.
    TickInit();

    // Initialize VS1063a
    InitVLSI();

    // Initialize GPIO
    GPIOInit();



    // Initialize command line interpreter
    cli = new CLI();
    // Instantiate commands
    cmds = new CommandsList(cli);
    cli->createFileListOfCommands();
    //delete cmds;

    if (config.console.console == CLI_MODE) {
        // Initialize Command Line Interpreter
        ////        if (InitCli() == FALSE)
        ////            FlashLight(150, 50, TRUE);
    } else {
        // Initialize Single Character Commands Interpreter
        if (InitSCC(config.console.console) == FALSE)
            FlashLight(150, 50, TRUE);
    }

#if defined(USB_INTERRUPT)
    if (isUSBEnabled())
        USBDeviceAttach();
#endif

    // New C++ initialization of Playback class
    ccp = new CCPlay();

    // Initialize play and record tasks
    PlayTaskInit();
    RecordTaskInit();

    // Update gpio state with successful message
    GpioUpdateOutputState(GPIO_BIT_MICRO_SD);

    // Now that all items are initialized, begin the co-operative
    // multitasking loop.  This infinite loop will continuously
    // execute all stack-related tasks, as well as your own
    // application's functions.  Custom functions should be added
    // at the end of this loop.
    // Note that this is a "co-operative mult-tasking" mechanism
    // where every task performs its tasks (whether all in one shot
    // or part of it) and returns so that other tasks can do their
    // job.
    // If a task needs very long time to do its job, it must be broken
    // down into smaller pieces so that other tasks can have CPU time.
    while (1) {

        // Service the WDT
        ClearWDT();

        // Manager of the console routine
        if (config.console.console == CLI_MODE) {
            cli->cliTaskHadler();
            //            CliHandler();
        } else {
            SCCHandler();
        }
        //commandsTask();

        // Manager of I2C commander receiver
        //        I2CHandler();

        // Manager of the recording routine
        if (play == PLAY_IDLE)
            rec = RecordTaskHandler();

        // Manager of the player routine
        if (rec == REC_IDLE) {
            ccp->playTaskHandler();
            ////            play = PlayTaskHandler();
        }

        // BlinkLed
        if (play == REC_IDLE && rec == PLAY_IDLE)
            Toggle1Second();

        // GPIO Output Task handler
        GPIOOutputTaskHandler();
        // GPIO Output Task handler
        GPIOInputTaskHandler();


#if defined(USB_POLLING)
        // Check bus status and service USB interrupts.
        if (isUSBEnabled())
            USBDeviceTasks(); // Interrupt or polling method.  If using polling, must call
        // this function periodically.  This function will take care
        // of processing and responding to SETUP transactions
        // (such as during the enumeration process when you first
        // plug in).  USB hosts require that USB devices should accept
        // and process SETUP packets in a timely fashion.  Therefore,
        // when using polling, this function should be called
        // regularly (such as once every 1.8ms or faster** [see
        // inline code comments in usb_device.c for explanation when
        // "or faster" applies])  In most cases, the USBDeviceTasks()
        // function does not take very long to execute (ex: <100
        // instruction cycles) before it returns.
#endif

        if ((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1)) {

        } else {
            // USB CDC low level handler, if the USB cable is connected and correctly enumerated
            USBCDCCustomTaskHandler();
            CDCTxService();
            MSDTasks();
        }

    }
}
