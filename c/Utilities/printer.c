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

#include "Utilities/printer.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "Utilities/Config.h"
#include "Utilities/Uart.h"
#include "USB/usb_function_cdc.h"
#include "Utilities/Uart.h"
#include "Utilities/GPIO.h"
#include "I2CSlave.h"
#include "Delay/Delay.h"

PRINTER_CONFIG pri;

void InitPinter(void) {
    // Reset alternating buffer
    pri.alt = 0;
}

int __putc(char c) {
    // print a single character
    consoleWrite(&c, 1);
}

//int __puts(const char *p) {
//
//    // print a string
//    while (*p != 0)
//        __putc(*p++);
//}

int __printf(const char * fmt, ...) {

    va_list ap;
    int len, sent, retry;
    char *p;

    retry = 20;
    len = sent = 0;
    p = pri.txBuf[pri.alt++ % PRI_BUF_ALT_DIM];

    // If the user disable both UART and USB serial console do anything
    if (isUARTEnabled() || isUSBEnabled() || isI2CEnabled()) {
        va_start(ap, fmt);
        len = vsnprintf(p, PRINTER_BUFFER_SIZE, fmt, ap);
        va_end(ap);

        // Print on UART, USB or I2C port
        do {
            sent += consoleWrite(&p[sent], len - sent);
        } while (sent < len && retry--);
    }
    return sent;
}

char returnLineVerbose[] = "\r\n>";

int verbosePrintf(int level, const char * fmt, ...) {

    va_list ap;
    int n;
    char buf[1024];

    if (level <= config.console.verbose) {
        // If the user disable both UART and USB serial console do anything
        if (isUARTEnabled() || isUSBEnabled() || isI2CEnabled()) {
            va_start(ap, fmt);
            n = vsnprintf(buf, sizeof (buf), fmt, ap);
            va_end(ap);

            // Print on UART, USB or I2C port
            consoleWrite(buf, n);
            consoleWrite(returnLineVerbose, sizeof (returnLineVerbose));
            return n;
        }
    }
    // TODO write to log file

    return 0;
}

uint16_t consoleWrite(uint8_t *buffer, uint16_t count) {

    if (config.console.port == 0) {
        // Print on UART port
        return UartWrite(buffer, count); // TODO Should work with UartWriteDirectly
    } else if (config.console.port == 1) {
        // Print on USB port
        return USBWrite(buffer, count);
    } else {
        // Print on I2C port
        return I2CWrite(buffer, count);
    }
}

uint16_t consoleRead(uint8_t *buffer, uint16_t count) {

    if (config.console.port == 0) {
        return UartRead(buffer, count);
    } else if (config.console.port == 1) {
        return USBRead(buffer, count);
    } else {
        return I2CRead(buffer, count);
    }
}

int Config(int argc, char **argv) {

    if (argc < 2) {
        printf("Serial-port: ");
        if (config.console.port == 0)
            printf("UART\r\n");
        else if (config.console.port == 1)
            printf("USB\r\n");
        else if (config.console.port == 2)
            printf("I2C\r\n");
    } else if (argc == 2) {
        // Check first the existence of the passed file
        config.console.port = atoimm(argv[1], 0, 2, 0);
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int Verbose(int argc, char **argv) {

    if (argc < 2) {
        printf("Verbose: %d\r\n", config.console.verbose);
    } else if (argc == 2) {
        // Check first the existence of the passed file
        config.console.verbose = atoimm(argv[1], 0, 3, 2);
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}





