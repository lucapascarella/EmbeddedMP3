/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        Logging.c
 * Dependencies:    Logging.h
 * Processor:       PIC32MX250F128B
 * Compiler:        Microchip XC32 v1.11a or higher
 * Company:         LP Systems
 * Author:	    Luca Pascarella luca.pascarella@gmail.com
 * Web Site:        www.lucapascarella.it
 *
 * Software License Agreement
 *
 * Copyright (C) 2012-2013 LP Systems  All rights reserved.
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
 * File Description: Encoder and Decoder state finite machine
 * Change History: In progress
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release (1 September 2013, 16.00)
 *
 ********************************************************************/

#include "Utilities/Logging.h"
#include "Utilities/Config.h"
#include "Utilities/RTCC.h"

FIL logFile;

int logCount;

BOOL openLogFile() {

    FRESULT fres;

    if ((fres = f_open(&logFile, "log.txt", FA_READ | FA_WRITE | FA_OPEN_ALWAYS)) != FR_OK)
        return FALSE;

    // Opens the file if it is existing. If not, a new file is created.
    // To append data to the file, use f_lseek() function after file open in this method.
    put_rc(f_lseek(&logFile, f_size(&logFile)));

    f_printf(&logFile, "\r\n");
    put_rc(f_sync(&logFile));

    logCount = 0;
    writeLogFile(LOG_NONE, "Logging.c", "The logging was started successful to level: %d.", config.console.log);

    return TRUE;
}

void closeLogFile() {

    // Flush the content on the micro SD
    put_rc(f_sync(&logFile));
    // Close the file
    put_rc(f_close(&logFile));

}

void writeLogFile(int severity, char* filename, char* format, ...) {

    va_list argp;
    char text[2048], sTime[32];
    int len;

    // Makes a string from the variable input arguments
    va_start(argp, format);
    vsnprintf(text, sizeof (text), format, argp);
    va_end(argp);

    // If "severity" debug level is set sufficiently over the "Config level" append the string to the file
    if (severity <= config.console.log) {
        //if (logFile.) {
        // Get and format the current time and date
        len = strftime(sTime, sizeof (sTime), "[%d/%m/%Y %H:%M:%S", rtccGetTime());

        f_printf(&logFile, "%s C=%d N=%s S=%d] %s\r\n", sTime, logCount++, filename, severity, text);
        // Flush the content on the micro SD
        put_rc(f_sync(&logFile));
        //}
    }
}

