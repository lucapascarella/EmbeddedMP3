/*********************************************************************
 *
 *	Hardware specific definitions for:
 *    - PIC32 Ethernet Starter Kit
 *    - PIC32MX795F512L
 *    - Internal 10/100 Ethernet MAC with National DP83848 10/100 PHY
 *
 *********************************************************************
 * FileName:        HardwareProfile.h
 * Dependencies:    Compiler.h
 * Processor:       PIC32
 * Compiler:        Microchip C32 v1.11 or higher
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
 * Author               Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Howard Schlunder		09/16/2010	Regenerated for specific boards
 ********************************************************************/
#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include "Compiler.h"


// Clock frequency values
// These directly influence timed events using the Tick module.  They also are used for UART and SPI baud rate generation.
#define GetSystemClock()		(50000000ul)		// Hz
#define GetInstructionClock()           (GetSystemClock()/1)	// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Might need changing if using Doze modes.
#define GetPeripheralClock()            (GetSystemClock()/1)	// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Divisor may be different if using a PIC32 since it's configurable.

// RTCC use
#define TOGGLES_PER_SEC                 1000
#define CORE_TICK_RATE                  (GetSystemClock()/2/TOGGLES_PER_SEC)
#define TIMER_2_PERIOD                  (GetSystemClock()/64/TOGGLES_PER_SEC)

// Hardware I/O pin mappings

// LEDs
#define LEDs_TRIS			(TRISBbits.TRISB4)	// Ref LEDs
#define LED_RED_ON()                    {TRISBbits.TRISB4 = 0; LATBbits.LATB4 = 1;}
#define LED_BLUE_ON()                   {TRISBbits.TRISB4 = 0; LATBbits.LATB4 = 0;}
#define LEDs_OFF()                      (TRISBbits.TRISB4 = 1)
#define LEDs_TOGGLE()                   {TRISBbits.TRISB4 = 0; mPORTBToggleBits(BIT_4);}

#define LED_RED_IO()                    (LATBbits.LATB4)
#define LED_BLUE_IO()                   (LATBbits.LATB4)

#define GPIO_0_TRIS                     TRISBbits.TRISB8    // SCL1
#define GPIO_1_TRIS                     TRISBbits.TRISB9    // SDA1
#define GPIO_2_TRIS                     TRISAbits.TRISA0    // AN0/IO3
#define GPIO_3_TRIS                     TRISAbits.TRISA1    // AN1/IO4
#define GPIO_4_TRIS                     TRISBbits.TRISB1    // U2RX
#define GPIO_5_TRIS                     TRISBbits.TRISB0    // U2TX
#define GPIO_6_TRIS                     TRISBbits.TRISB10   // USB_P
#define GPIO_7_TRIS                     TRISBbits.TRISB11   // USB_N

#define GPIO_0_INPUT                    PORTBbits.RB8
#define GPIO_1_INPUT                    PORTBbits.RB9
#define GPIO_2_INPUT                    PORTAbits.RA0
#define GPIO_3_INPUT                    PORTAbits.RA1
#define GPIO_4_INPUT                    PORTBbits.RB1
#define GPIO_5_INPUT                    PORTBbits.RB0
#define GPIO_6_INPUT                    PORTBbits.RB10
#define GPIO_7_INPUT                    PORTBbits.RB11

#define GPIO_0_OUTPUT                   LATBbits.LATB8
#define GPIO_1_OUTPUT                   LATBbits.LATB9
#define GPIO_2_OUTPUT                   LATAbits.LATA0
#define GPIO_3_OUTPUT                   LATAbits.LATA1
#define GPIO_4_OUTPUT                   LATBbits.LATB1
#define GPIO_5_OUTPUT                   LATBbits.LATB0
#define GPIO_6_OUTPUT                   LATBbits.LATB10
#define GPIO_7_OUTPUT                   LATBbits.LATB11

#define GPIO_0_TOGGLE()                 mPORTBToggleBits(BIT_8)
#define GPIO_1_TOGGLE()                 mPORTBToggleBits(BIT_9)
#define GPIO_2_TOGGLE()                 mPORTAToggleBits(BIT_0)
#define GPIO_3_TOGGLE()                 mPORTAToggleBits(BIT_1)
#define GPIO_4_TOGGLE()                 mPORTBToggleBits(BIT_1)
#define GPIO_5_TOGGLE()                 mPORTBToggleBits(BIT_0)
#define GPIO_6_TOGGLE()                 mPORTBToggleBits(BIT_10)
#define GPIO_7_TOGGLE()                 mPORTBToggleBits(BIT_11)

#define GPIO_0_PULL_UP                  CNPUBbits.CNPUB8
#define GPIO_1_PULL_UP                  CNPUBbits.CNPUB9
#define GPIO_2_PULL_UP                  CNPUAbits.CNPUA0
#define GPIO_3_PULL_UP                  CNPUAbits.CNPUA1
#define GPIO_4_PULL_UP                  CNPUBbits.CNPUB1
#define GPIO_5_PULL_UP                  CNPUBbits.CNPUB0
#define GPIO_6_PULL_UP                  CNPUBbits.CNPUB10
#define GPIO_7_PULL_UP                  CNPUBbits.CNPUB11

#define GPIO_0_PULL_DOWN                CNPDBbits.CNPDB8
#define GPIO_1_PULL_DOWN                CNPDBbits.CNPDB9
#define GPIO_2_PULL_DOWN                CNPDAbits.CNPDA0
#define GPIO_3_PULL_DOWN                CNPDAbits.CNPDA1
#define GPIO_4_PULL_DOWN                CNPDBbits.CNPDB1
#define GPIO_5_PULL_DOWN                CNPDBbits.CNPDB0
#define GPIO_6_PULL_DOWN                CNPDBbits.CNPDB10
#define GPIO_7_PULL_DOWN                CNPDBbits.CNPDB11

#define GPIO_0_CHANGE_NOTICE            CNENBbits.CNIEB8
#define GPIO_1_CHANGE_NOTICE            CNENBbits.CNIEB9
#define GPIO_2_CHANGE_NOTICE            CNENAbits.CNIEA0
#define GPIO_3_CHANGE_NOTICE            CNENAbits.CNIEA1
#define GPIO_4_CHANGE_NOTICE            CNENBbits.CNIEB1
#define GPIO_5_CHANGE_NOTICE            CNENBbits.CNIEB0
#define GPIO_6_CHANGE_NOTICE            CNENBbits.CNIEB10
#define GPIO_7_CHANGE_NOTICE            CNENBbits.CNIEB11

#define GPIO_0_SCL                      0
#define GPIO_1_SDA                      1
#define GPIO_2_AN0                      2
#define GPIO_3_AN1                      3
#define GPIO_4_RX                       4
#define GPIO_5_TX                       5
#define GPIO_6_USB_P                    6
#define GPIO_7_USB_N                    7

#define GPIO_NUMBERS                    8


// UART configuration (not too important since we don't have a UART
// connector attached normally, but needed to compile if the STACK_USE_UART
// or STACK_USE_UART2TCP_BRIDGE features are enabled.
#define UART_TX_TRIS                    (TRISBbits.TRISB0)      // Testata ed è OK
#define UART_RX_TRIS                    (TRISBbits.TRISB1)      // Testata ed è OK

#ifndef INPUT
#define INPUT                           1
#endif

#ifndef OUTPUT
#define OUTPUT                          0
#endif



// VS1063
#define MP3_XCS_O                       (LATBbits.LATB2)
#define MP3_XCS_TRIS                    (TRISBbits.TRISB2)
//#define MP3_XCS_ADC                   (ANSELBbits.ANSB2)

#define MP3_XDCS_O                      (LATBbits.LATB13)
#define MP3_XDCS_TRIS                   (TRISBbits.TRISB13)
//#define MP3_XDCS_ADC                  (ANSELBbits.ANSB)

#define MP3_XRESET_O                    (LATBbits.LATB15)
#define MP3_XRESET_TRIS                 (TRISBbits.TRISB15)

#define MP3_DREQ_I                      (PORTBbits.RB7)
#define MP3_DREQ_TRIS                   (TRISBbits.TRISB7)

#define MP3_SCK_TRIS                    (SPICLOCK)
#define MP3_SDI_TRIS                    (SPIIN)
#define MP3_SDO_TRIS                    (SPIOUT)

#define MP3_SPICON1                     (SPICON1)
#define MP3_SPISTAT                     (SPISTAT)
#define MP3_SPISTATbits                 (SPISTATbits)
#define MP3_SPICON2                     (SPICON2)
//#define MP3_SPI_IF                    (IFS0bits.SPI1IF)
#define MP3_SPI_RBF                     (SPISTAT_RBF)
#define MP3_SPIBUF                      (SPIBUF)


/*******************************************************************/
/******** USB stack hardware selection options *********************/
/*******************************************************************/
//This section is the set of definitions required by the MCHPFSUSB
//  framework.  These definitions tell the firmware what mode it is
//  running in, and where it can find the results to some information
//  that the stack needs.
//These definitions are required by every application developed with
//  this revision of the MCHPFSUSB framework.  Please review each
//  option carefully and determine which options are desired/required
//  for your application.

//#define USE_SELF_POWER_SENSE_IO
#define tris_self_power     TRISAbits.TRISA2    // Input
#define self_power          1

//#define USE_USB_BUS_SENSE_IO
#define tris_usb_bus_sense  TRISAbits.TRISA2    // Input
#define USB_BUS_SENSE       1




/*******************************************************************/
/******** MDD File System selection options ************************/
/*******************************************************************/
//#define USB_USE_MSD
//#define USE_SD_INTERFACE_WITH_SPI
//#define USE_PIC32
//#define USE_32BIT

//SPI Configuration
//#define SPI_START_CFG_1                 (PRI_PRESCAL_64_1 | SEC_PRESCAL_8_1 | MASTER_ENABLE_ON | SPI_CKE_ON | SPI_SMP_ON)
//#define SPI_START_CFG_2                 (SPI_ENABLE)

// Define the SPI frequency
#define SPI_BRG_24MHZ                   (0x00)      // With 48MHz clock
#define SPI_BRG_12MHZ                   (0x01)      // With 48MHz clock
#define SPI_BRG_8MHZ                    (0x02)      // With 48MHz clock
#define SPI_BRG_1MHZ                    (0x17)      // With 48MHz (0x13 with 40Mhz)


#define SD_CS_O                         (LATBbits.LATB3)
#define SD_CS_I                         (PORTBbits.RB3)
#define SD_CS_TRIS                      (TRISBbits.TRISB3)

#define MEDIA_SOFT_DETECT


// Registers for the SPI module you want to use
#define SPICON1                         SPI1CON
#define SPICON2                         SPI1CON2
#define SPISTAT                         SPI1STAT
#define SPIBUF                          SPI1BUF
#define SPISTAT_RBF                     SPI1STATbits.SPIRBF
#define SPICON1bits                     SPI1CONbits
#define SPISTATbits                     SPI1STATbits
#define SPIENABLE                       SPICON1bits.ON
#define SPIBRG                          SPI1BRG

#define SPI_CHANNEL                     SPI_CHANNEL1

// Tris pins for SCK/SDI/SDO lines
#define SPICLOCK                        TRISBbits.TRISB14
#define SPIIN                           TRISBbits.TRISB5
#define SPIOUT                          TRISAbits.TRISA4
#define SPIIN_PULLUP                    (CNPUBbits.CNPUB8)


#endif // #ifndef HARDWARE_PROFILE_H
