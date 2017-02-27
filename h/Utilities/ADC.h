/*********************************************************************
 *
 *  ADC Low Level Subroutines
 *
 *********************************************************************
 * FileName:        ADC.h
 * Dependencies:    Compiler.h GenericTypeDefs.h HardwareProfile.h
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
#ifndef ADC_H
#define	ADC_H

#include "HardwareProfile.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "GPIO.h"

#define isADCEnabled()      (config.gpio[GPIO_2_AN0].mode == GPIO_S_ANALOG && config.gpio[GPIO_3_AN1].mode == GPIO_S_ANALOG)

void Adc(int argc, char **argv);



#endif	/* ADC_H */

