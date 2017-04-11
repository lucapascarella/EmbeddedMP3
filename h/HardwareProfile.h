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

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H


// Clock frequency values
// These directly influence timed events using the Tick module.  They also are used for UART and SPI baud rate generation.
#define GetSystemClock()                (50000000ul)		// Hz
#define GetInstructionClock()           (GetSystemClock()/1)	// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Might need changing if using Doze modes.
#define GetPeripheralClock()            (GetSystemClock()/1)	// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Divisor may be different if using a PIC32 since it's configurable.

// RTCC use
#define TOGGLES_PER_SEC                 1000
#define CORE_TICK_RATE                  (GetSystemClock()/2/TOGGLES_PER_SEC)
#define TIMER_2_PERIOD                  (GetSystemClock()/64/TOGGLES_PER_SEC)


// Hardware I/O pin mappings

// LEDs
#define LEDs_TRIS                       (TRISBbits.TRISB4)	// Ref LEDs
#define LED_RED_ON()                    {TRISBCLR = 0x10; LATBSET = 0x10;}
#define LED_BLUE_ON()                   {TRISBCLR = 0x10; LATBCLR = 0x10;}
#define LEDs_OFF()                      (TRISBSET = 0x10)
#define LEDs_TOGGLE()                   {TRISBCLR = 0x10; LATBINV = 0x10;}

//#define LED_RED_IO()                    (LATBbits.LATB4)
//#define LED_BLUE_IO()                   (LATBbits.LATB4)

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

// DMA 3 for REC purpose
#define UART_DMA_CHANNEL                DMA_CHANNEL2
#define UART_DMA_WORKING()              (DCH2INTbits.CHBCIF == FALSE)
#define UART_DMA_CLR_BTC()              (DCH2INTCLR = 0x00FF)
#define UART_DMA_SET_BTC()              (DCH2INTSET = 0x0008)


#define MEM_MATCH_DMA_CHANNEL           DMA_CHANNEL0
#define MEM_MATCH_DCHxSSA               DCH0SSA         // Source start address
#define MEM_MATCH_DCHxDSA               DCH0DSA         // Destination start address
#define MEM_MATCH_DCHxSSIZ              DCH0SSIZ        // Source Size
#define MEM_MATCH_DCHxDSIZ              DCH0DSIZ        // Destination Size
#define MEM_MATCH_DCHxSPTR              DCH0SPTR        // Source Pointer
#define MEM_MATCH_DCHxDPTR              DCH0DPTR        // Destination Pointer
#define MEM_MATCH_DCHxCSIZ              DCH0CSIZ        // Cell Size
#define MEM_MATCH_DCHxCPTR              DCH0CPTR        // Cell Pointer
#define MEM_MATCH_DCHxDAT               DCH0DAT         // Data to be matched
#define MEM_MATCH_DMA_WORKING()         (DCH0INTbits.CHBCIF == FALSE)
#define MEM_MATCH_DMA_CLR_BTC()         (DCH0INTCLR = 0x00FF)
#define MEM_MATCH_DMA_SET_BTC()         (DCH0INTSET = 0x0008)

#define MEM_TO_MEM_DMA_CHANNEL          DMA_CHANNEL1
#define MEM_TO_MEM_DCHxSSA              DCH1SSA         // Source start address
#define MEM_TO_MEM_DCHxDSA              DCH1DSA         // Destination start address
#define MEM_TO_MEM_DCHxSSIZ             DCH1SSIZ        // Source Size
#define MEM_TO_MEM_DCHxDSIZ             DCH1DSIZ        // Destination Size
#define MEM_TO_MEM_DCHxSPTR             DCH1SPTR        // Source Pointer
#define MEM_TO_MEM_DCHxDPTR             DCH1DPTR        // Destination Pointer
#define MEM_TO_MEM_DCHxCSIZ             DCH1CSIZ        // Cell Size
#define MEM_TO_MEM_DCHxCPTR             DCH1CPTR        // Cell Pointer
#define MEM_TO_MEM_DCHxDAT              DCH1DAT         // Data to be matched
#define MEM_TO_MEM_DMA_WORKING()        (DCH1INTbits.CHBCIF == FALSE)
#define MEM_TO_MEM_DMA_CLR_BTC()        (DCH1INTCLR = 0x00FF)
#define MEM_TO_MEM_DMA_SET_BTC()        (DCH1INTSET = 0x0008)

#ifndef INPUT
#define INPUT                           1
#endif

#ifndef OUTPUT
#define OUTPUT                          0
#endif



// VS1063
//#define MP3_XCS_O                       (LATBbits.LATB2)
#define MP3_XCS_TRIS                    (TRISBbits.TRISB2)
#define MP3_XCS_O_LOW()                 (LATBCLR = 0x0004)
#define MP3_XCS_O_HIGH()                (LATBSET = 0x0004)

//#define MP3_XDCS_O                      (LATBbits.LATB13)
#define MP3_XDCS_TRIS                   (TRISBbits.TRISB13)
#define MP3_XDCS_O_LOW()                (LATBCLR = 0x2000)
#define MP3_XDCS_O_HIGH()               (LATBSET = 0x2000)

//#define MP3_XRESET_O                    (LATBbits.LATB15)
#define MP3_XRESET_TRIS                 (TRISBbits.TRISB15)
#define MP3_XRESET_O_LOW()              (LATBCLR = 0x8000)
#define MP3_XRESET_O_HIGH()             (LATBSET = 0x8000)

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
#define SPI_BRG_25MHZ                   (0x00)      // With 50MHz clock
#define SPI_BRG_12_5MHZ                 (0x01)      // With 50MHz clock
#define SPI_BRG_8_33MHZ                 (0x02)      // With 50MHz clock
#define SPI_BRG_1MHZ                    (0x18)      // With 50MHz (0x13 with 40Mhz)


//#define SD_CS_O                         (LATBbits.LATB3)
//#define SD_CS_I                         (PORTBbits.RB3)
#define SD_CS_TRIS                      (TRISBbits.TRISB3)
#define SD_CS_CLR                       (LATBCLR = 0x08)
#define SD_CS_SET                       (LATBSET = 0x08)
#define SD_CS_INV                       (LATBINV = 0x08)

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

// DMA 3 for REC purpose
#define REC_DMA_CHANNEL                 DMA_CHANNEL3
#define REC_DMA_WORKING()               (DCH3INTbits.CHBCIF == FALSE)
#define REC_DMA_CLR_BTC()               (DCH3INTCLR = 0x00FF)
#define REC_DMA_SET_BTC()               (DCH3INTSET = 0x0008)

#endif // #ifndef HARDWARE_PROFILE_H
