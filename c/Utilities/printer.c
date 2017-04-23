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

#include <Cpp/c/stdarg.h>

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
#include "Utilities/CustomFunctions.h"

PRINTER_CONFIG pri;

void InitPinter(void) {
    // Reset alternating buffer
    pri.alt = 0;
}

int __putc(char c) {
    // print a single character

    return consoleWrite(&c, 1);
    return 0;
}

int consolePrint(uint8_t *buffer, uint16_t count) {

    va_list ap;
    int sent, retry;
    char *p;

    retry = 20;
    sent = 0;
    p = pri.txBuf[pri.alt++ % PRI_BUF_ALT_DIM];
    custom_memcpy(p, buffer, count);
    // Print on UART, USB or I2C port
    do {
        sent += consoleWrite(&p[sent], count - sent);
    } while (sent < count && retry--);
    return sent;
}

int __printf(const char * fmt, ...) {

    va_list ap;
    int len, sent, retry;
    char *p;

    retry = 20;
    len = sent = 0;
    p = pri.txBuf[pri.alt++ % PRI_BUF_ALT_DIM];

    // If the user disable both UART and USB serial console do anything
    va_start(ap, fmt);
    len = vsnprintf(p, PRINTER_BUFFER_SIZE, fmt, ap);
    va_end(ap);
    // Print on UART, USB or I2C port
    do {
        sent += consoleWrite(&p[sent], len - sent);
    } while (sent < len && retry--);
    return sent;
}

//char returnLineVerbose[] = "\r\n>";

int verbosePrintfVaList(int level, const char * fmt, va_list args) {

    int len, sent, retry;
    char *p;

    retry = 20;
    len = sent = 0;
    p = pri.txBuf[pri.alt++ % PRI_BUF_ALT_DIM];

    if (level <= config.console.verbose) {
        consoleWrite("\r\n", 2);
        // If the user disable both UART and USB serial console do anything
        //va_start(ap, fmt);
        len = vsnprintf(p, PRINTER_BUFFER_SIZE, fmt, args);
        //va_end(ap);
        // Print on UART, USB or I2C port
        do {
            sent += consoleWrite(&p[sent], len - sent);
        } while (sent < len && retry--);
    }
    // TODO write to log file
    return sent;
}

int verbosePrintf(int level, const char * fmt, ...) {

    int len;
    va_list ap;

    va_start(ap, fmt);
    verbosePrintfVaList(level, fmt, ap);
    va_end(ap);

    return len;
    //
    //    int len, sent, retry;
    //    char *p;
    //
    //    retry = 20;
    //    len = sent = 0;
    //    p = pri.txBuf[pri.alt++ % PRI_BUF_ALT_DIM];
    //
    //    if (level <= config.console.verbose) {
    //        consoleWrite("\r\n", 2);
    //        // If the user disable both UART and USB serial console do anything
    //        va_start(ap, fmt);
    //        len = vsnprintf(p, PRINTER_BUFFER_SIZE, fmt, ap);
    //        va_end(ap);
    //        // Print on UART, USB or I2C port
    //        do {
    //            sent += consoleWrite(&p[sent], len - sent);
    //        } while (sent < len && retry--);
    //    }
    //    // TODO write to log file
    //    return sent;
}

uint16_t consoleWrite(uint8_t *buffer, uint16_t count) {
    if (count != 0 && (isUARTEnabled() || isUSBEnabled() || isI2CEnabled()))
        if (config.console.port == 0)
            return UartWrite(buffer, count); // TODO Should work with UartWriteDirectly
        else if (config.console.port == 1)
            return USBWrite(buffer, count);
        else if (config.console.port == 2)
            return I2CWrite(buffer, count);
    return 0;
}

uint16_t consoleRead(uint8_t *buffer, uint16_t count) {
    if (count != 0 && (isUARTEnabled() || isUSBEnabled() || isI2CEnabled()))
        if (config.console.port == 0)
            return UartRead(buffer, count);
        else if (config.console.port == 1)
            return USBRead(buffer, count);
        else if (config.console.port == 2)
            return I2CRead(buffer, count);
    return 0;
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





