/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        Logging.c
 * Dependencies:    Logging.h, RTCC.h, Config.h
 * Processor:       PIC32MX270F256B
 * Compiler:        Microchip XC32 v1.33 or higher
 * Company:         LP Systems
 * Author:          Luca Pascarella luca.pascarella@gmail.com
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
 * 1.1   Added put_rc() function (12 August 2015)
 *
 ********************************************************************/

#include "Utilities/Logging.h"
#include "Utilities/Config.h"
#include "Utilities/RTCC.h"

const char logging_file[] = "Logging.c";

FIL logFile;
int logCount;

BOOL openLogFile() {

    // Opens the file if it is existing. If not, a new file is created.
    if (!put_rc(f_open(&logFile, "log.txt", FA_READ | FA_WRITE | FA_OPEN_ALWAYS)))
        return FALSE;

    // To append data to the file, use f_lseek() function after file open in this method.
    if (!put_rc(f_lseek(&logFile, f_size(&logFile))))
        return FALSE;

    // Append newline to log file
    f_printf(&logFile, "\r\n");
    if (!put_rc(f_sync(&logFile)))
        return FALSE;

    // Reset the log counter
    logCount = 0;
    writeToLogFile(LOG_NONE, logging_file, "The logging was started successful to level: %d.", config.console.log);

    return TRUE;
}

void closeLogFile() {

    // Flush the content of the log file into micro SD
    put_rc(f_sync(&logFile));
    // Close log file
    put_rc(f_close(&logFile));

}

void writeToLogFile(int severity, const char* filename, char* format, ...) {

    va_list argp;
    char varText[2048], sTime[32];
    int len;
    WORD year;
    BYTE mon, day, hour, mins, sec;

    // If "severity" debug level is set sufficiently over the "Config level" append the string to the file
    if (severity <= config.console.log) {
        // Makes a string from the variable input arguments
        va_start(argp, format);
        vsnprintf(varText, sizeof (varText), format, argp);
        va_end(argp);

        // Get and format the current time and date
        //len = strftime(sTime, sizeof (sTime), "%d/%m/%Y %H:%M:%S", rtccGetDateAndTimeTM());

        rtccGetDateAndTime(&year, &mon, &day, &hour, &mins, &sec);
        len = snprintf(sTime, sizeof (sTime), "%02d%02d%04d %02d%02d%02d", day, mon, year, hour, mins, sec);

        // Print the log message within log file
        f_printf(&logFile, "%s C=%d N=%s S=%d] %s\r\n", sTime, logCount++, filename, severity, varText);
        // Flush the content on the micro SD
        put_rc(f_sync(&logFile));
    }
}

