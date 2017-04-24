/*********************************************************************
 *
 *  ADC Low Level Subroutines
 *
 *********************************************************************
 * FileName:        ADC.c
 * Dependencies:    ADC.h
 * Processor:       PIC32MX270F256B
 * Compiler:        Microchip XC32 v1.32 or higher
 * Company:         LP Systems
 * Author:	    Luca Pascarella www.lucapascarella.it
 *
 * Software License Agreement
 *
 * Copyright (C) 2012-2014 LP Systems  All rights reserved.
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
 * File Description: Low level subroutine entry point to interact with ADC peripheral
 * Change History: In progress
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release
 *
 ********************************************************************/

#include "Utilities/ADC.h"
#include "Utilities/printer.h"

#include "Utilities/Config.h"

// Define setup parameters for OpenADC10 function
// Turn module on | Ouput in integer format | Trigger mode auto | Enable autosample
#define config1     ADC_FORMAT_INTG | ADC_CLK_AUTO | ADC_AUTO_SAMPLING_ON
// ADC ref external | Disable offset test | Disable scan mode | Perform 2 samples | Use dual buffers | Use alternate mode
#define config2     ADC_VREF_AVDD_AVSS | ADC_OFFSET_CAL_DISABLE | ADC_SCAN_OFF | ADC_SAMPLES_PER_INT_2 | ADC_ALT_BUF_ON | ADC_ALT_INPUT_ON
// Use ADC internal clock | Set sample time
#define config3     ADC_CONV_CLK_INTERNAL_RC | ADC_SAMPLE_TIME_15
// Do not assign channels to scan
#define configscan  SKIP_SCAN_AN0 | SKIP_SCAN_AN1 //SKIP_SCAN_ALL

// AN0/RA0/PGD/IO2 & AN1/RA1/PGC/IO3
#define configport  ENABLE_AN0_ANA | ENABLE_AN1_ANA

#define OpenADC10_A(config1, config2, config3, configport, configscan) (mPORTASetPinsAnalogIn(configport), AD1CSSL = ~(configscan), AD1CON3 = (config3), AD1CON2 = (config2), AD1CON1 = (config1) )

BOOL InitADC(void) {



    // Ensure the ADC is off before setting the configuration
    CloseADC10();

    // Configure to sample AN0/RA0/PGD/IO2 & AN1/RA1/PGC/IO3
    // Use ground as neg ref for A | use AN4 for input A | use ground as neg ref for A | use AN5 for input B
    SetChanADC10(ADC_CH0_NEG_SAMPLEA_NVREF | ADC_CH0_POS_SAMPLEA_AN0 | ADC_CH0_NEG_SAMPLEB_NVREF | ADC_CH0_POS_SAMPLEB_AN1);


    // Configure ADC using the parameters defined above
    OpenADC10_A(config1, config2, config3, configport, configscan);

    // Enable the ADC
    EnableADC10();
}

int Adc(int argc, char **argv) {

    unsigned int an0, an1;
    unsigned int offset; // Buffer offset to point to the base of the idle buffer

    if (!isADCEnabled()) {
        printf("ADC is not activated in the config file\r\n");
        return;
    }

    if (argc == 1) {
        while (!mAD1GetIntFlag()) {
            // Wait for the first conversion to complete so there will be vaild data in ADC result registers
        }

        // Determine which buffer is idle and create an offset
        offset = 8 * ((~ReadActiveBufferADC10() & 0x01));

        // Read the result of temperature sensor conversion from the idle buffer
        an0 = ReadADC10(offset);

        // Read the result of pot conversion from the idle buffer
        an1 = ReadADC10(offset + 1);

        mAD1ClearIntFlag();

        printf("AN 0: %d\r\n", an0);
        printf("AN 1: %d\r\n", an1);
    } else if (argc == 2) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 3) {

    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}