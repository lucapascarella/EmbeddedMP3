/*********************************************************************
 *
 *  Command Line Interpreter
 *
 *********************************************************************
 * FileName:        CommandLineInterpreter.c
 * Dependencies:    CommandLineInterpreter.h
 *                  HardwareProfile.h
 *                  Compiler.h
 *                  GenericTypeDefs.h
 * Processor:       PIC32MX250F128B
 * Compiler:        Microchip XC32 v1.21 or higher
 * Company:         LP Systems
 * Author:          Luca Pascarella www.lucapascarella.it
 *
 * Software License Agreement
 *
 * Copyright (C) 2012-2013 All rights reserved.
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
 * File Description:
 * This file contains the routines to manage a command line interpeter
 *
 * Change History:
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release
 *
 ********************************************************************/


#include "CommandLineInterpreter.h"
#include "Utilities/Uart.h"
#include "Delay/Delay.h"
#include "Utilities/Config.h"
#include "Utilities/printer.h"
#include "Utilities/GPIO.h"
#include "FatFS/ff.h"
#include "Commands.h"
#include "Utilities/RTCC.h"
#include "Utilities/ADC.h"

// Declaration of initialized global variables
const COMMAND_SELECTOR commands[] = {
    "adc ", Adc, // Do a ADC conversion
    "bass ", Bass, // Sets or returns the value of the low tones
    "bookmark ", Bookmark, // Manages bookmarks
    "cat ", Cat, // Print the contents of a textual file
    "cd ", ChangeDir, // Change the working directory
    "cfg ", Config, // Change the configuration values
    "clear ", Clear, // Clear the screen
    "cp ", Copy, // Copy a file
    "debug ", Debug, // Debug
    "ear ", EarSpeaker, // Gets and Sets the EarSpeaker Spatial Processing
    "fat ", FileSystem, // Show file system status
    "flash ", Flash, // Flash
    "fplay ", FPlay, // Sets or returns the incremetal speed execution
    "gpio ", Gpio, // Returns the GPIO state
    "help ", Help, // Returns the help informations
    "info ", Info, // Print the information of the current track
    "ls ", List, // Return the contents of the working directory
    "mkdir ", Mkdir, // Create a directory
    "mv ", Move, // Rename a file or directory
    "pause ", Pause, // Toggle pause
    "play ", Play, // Put a track in execution
    "playlist ", Playlist, // Playlist
    "rate ", RateTune, // Gets or sets the rate finetune        // get or set samplerate finetune in +-1ppm steps
    "reboot ", Reboot, // Reboot
    "rec ", Record, // Start recording
    "rtcc ", Rtcc, // Get and set real time clock and calendar
    "rm ", Delete, // Remove a file
    "rmdir ", DeleteDir, // Remove a directory
    "speed ", SpeedShifter, // Sets or returns the speed shift  // 11.11.7 Speed Shifter 0x1e1d
    "stone ", Semitone, // Gets and stes semitone value         // Combination of 'speed' and 'rate'
    "stop ", Stop, // Stop
    "treble ", Treble, // Sets or returns the value of the high tones
    "verbose ", Verbose, // Sets or returns the verbose level
    "version ", Version, // Returns the version information
    "vol ", Volume, // Sets or returns the value of the volume
};

#define NUMBER_OF_COMMANDS          sizeof(commands)/(sizeof(commands[0].name)+sizeof(commands[0].cmd))
#define MAX_TOKEN   20

int (*commandToCall)(int, char**);
int argc;
char *argv[MAX_TOKEN];

// Some useful constants
const char temporaryFileEntryList[] = "/lst.tmp";
const char temporaryFileCommands[] = "/cmd.tmp";

const char escape_arrow_left[] = {0x1B, 0x5B, 0x44, 0x00};
const char escape_arrow_right[] = {0x1B, 0x5B, 0x43, 0x00};
const char escape_arrow_up[] = {0x1B, 0x5B, 0x41, 0x00};
const char escape_arrow_down[] = {0x1B, 0x5B, 0x42, 0x00};

const char escape_home[] = {0x1B, 0x5B, 0x31, 0x7E, 0x00};
const char escape_page_up[] = {0x1B, 0x5B, 0x35, 0x7E, 0x00};
const char escape_page_down[] = {0x1B, 0x5B, 0x36, 0x7E, 0x00};
const char escape_end[] = {0x1B, 0x5B, 0x34, 0x7E, 0x00};

const char escape_del[] = {0x1B, 0x5B, 0x33, 0x7E, 0x00};

const char escape_clear_screen[] = {0x1B, 0x5B, 0x48, 0x1B, 0x5B, 0x4A, 0x00};
const char escape_clear_end_row[] = {0x1B, 0x5B, 0x4A, 0x00};

// Declaration of global variables
COMMAND_BUFFER cl;
COMMAND_ITERATOR cmd_iterator[NUMBER_OF_COMMANDS];
FIL fileLastCommands;

BOOL InitCli() {

    int i;

    // Reset all string buffer
    for (i = 0; i < CLI_MAX_BUF_SIZE; i++)
        cl.cmd[i] = cl.tmp[i] = '\0';

    // Reset other command line variables
    cl.cmdlen = cl.cmdi = cl.tmpi = cl.tmplen = 0;

    // Reset the commands iteretor
    for (i = 0; i < NUMBER_OF_COMMANDS; i++)
        cmd_iterator[i].cmd_len = strlen(commands[i].name) - 1;

    // Create two hidden files with the list of commands and entry list of current directory
    CliCreateFileListOfCommands();
    CliCreateFileListOfFilesEntry();

    // Creates a temporary file where it will put a list of last commands
    if (!put_rc(f_open(&fileLastCommands, "last.cmd", FA_READ | FA_WRITE | FA_OPEN_ALWAYS)))
        return FALSE;

    // Changes the properties of the temporary file to hide it
    put_rc(f_chmod("last.cmd", AM_HID, AM_HID));

    // Flush HIDDEN property
    put_rc(f_sync(&fileLastCommands));

    // Reset the last command indicator
    cl.nCmd = cl.lastCmd = 0;

    // CliPrintConsole
    CliReprintConsole();

    commandToCall = NULL;

    return TRUE;
}

void CliHandler(void) {

    int nCmd;
    char c;

    // Check if commandsTask is free to manage another command
    // and if a character is available, otherwise wait to next cycle
    if (commandToCall == NULL && CliGetCharFromConsole(&c)) {

        if (c == '\t') {
            // Horizontal Tab. Completes command
            CliCompleteCommand();
        } else if (c == '\0') {
            // Null character
            // Special char '\0' do nothing
        } else if (c == '\n') {
            // Line feed
            // Special char '\n' do nothing
        } else if (c == '\r') {
            // Carriage return
            printf("\r\n");
            // Check that it is a command or a name of a file
            if ((nCmd = CliSearchCommand(cl.cmd)) != -1) {
                // It is a command, print it
                CliPutLastCommandInFile();

                GpioUpdateOutputState(GPIO_BIT_CMD_OK);
                // Split the input in token and then sends argc and *argv[]
                argc = 0;
                argv[argc] = strtok(cl.cmd, " ");
                while (argv[argc] != NULL && argc < sizeof (argv))
                    argv[++argc] = strtok(NULL, " ");

                // Call the function associated with the command found and passes arguments argc and argv
                ////commands[nCmd].cmd(argc, argv);
                commandToCall = commands[nCmd].cmd;
                // Clear the command buffer
                CliClearCommand();
            } else {
                //if (strlen(cl.cmd) != 0) {
                if (cl.cmdlen != 0) {
                    GpioUpdateOutputState(GPIO_BIT_CMD_ERR);
                    // The command was not found
                    CliCommandNotFound(cl.cmd);
                    // Clear the command buffer and reprint the console indicator
                    CliClearCommand();
                }
                CliReprintConsole();
            }
        } else {
            // Any other character (printable or control), add it to the buffer and update sceen console
            CliAddCharAndUpdateConsole(c);
        }
    }
}

BOOL CliGetCharFromConsole(char *p) {

    static char tmp[10];
    static int countEscape;
    int i;

    while (ConsolRead(p, 1) != 0) {

        if (countEscape == 0 && *p != ESCAPE) {
            return TRUE;
        } else {
            tmp[countEscape++] = *p;
            if (countEscape > sizeof (tmp)) {
                for (i = 0; i < countEscape; i++)
                    tmp[i] = '\0';
                countEscape = 0;
                return FALSE;
            }
        }
    }

    if (countEscape > 2) {

        if (memcmp(tmp, escape_arrow_left, sizeof (escape_arrow_left)) == 0) {
            *p = ESCAPE_ARROW_LEFT;
        } else if (memcmp(tmp, escape_arrow_right, sizeof (escape_arrow_right)) == 0) {
            *p = ESCAPE_ARROW_RIGHT;
        } else if (memcmp(tmp, escape_arrow_up, sizeof (escape_arrow_up)) == 0) {
            *p = ESCAPE_ARROW_UP;
        } else if (memcmp(tmp, escape_arrow_down, sizeof (escape_arrow_down)) == 0) {
            *p = ESCAPE_ARROW_DOWN;
        } else if (memcmp(tmp, escape_home, sizeof (escape_home)) == 0) {
            *p = ESCAPE_HOME;
        } else if (memcmp(tmp, escape_page_up, sizeof (escape_page_up)) == 0) {
            *p = ESCAPE_PAGE_UP;
        } else if (memcmp(tmp, escape_page_down, sizeof (escape_page_down)) == 0) {
            *p = ESCAPE_PAGE_DOWN;
        } else if (memcmp(tmp, escape_end, sizeof (escape_end)) == 0) {
            *p = ESCAPE_END;
        } else if (memcmp(tmp, escape_del, sizeof (escape_del)) == 0) {
            *p = ESCAPE_DEL;
        } else {
            return FALSE;
        }
        for (i = 0; i < countEscape; i++)
            tmp[i] = '\0';
        countEscape = 0;
        return TRUE;
    }

    return FALSE;
}

BYTE CliCompleteCommand() {

    char *p, *fileName;

    // If there is nothing written in the buffer do nothing.
    if (cl.cmdlen == 0)
        return 0;

    // If in the buffer there is a space, search a file, otherwise search a command
    if ((p = strrchr(cl.cmd, ' ')) == NULL) {
        p = cl.cmd;
        fileName = (char*) temporaryFileCommands;
    } else {
        // p pointer refer to "strrchr(cl.cmd, ' ')"
        p++;
        fileName = (char*) temporaryFileEntryList;
    }

    // Complete the command by searching into selected file
    return CliCompleteCommandSearchInFile(fileName, p);
}

BYTE CliCompleteCommandSearchInFile(char *fileName, char *p) {

    extern FIL ftmp1;
    FIL *fp = &ftmp1;
    BOOL found;
    int len, occ, tmpLen;
    char match;

    // Search method (Files and Commands) unified
    if (!put_rc(f_open(fp, fileName, FA_READ))) {
        CliErrorOccurred("TAB");
        return FALSE;
    }

    found = FALSE;
    do {
        occ = 0;
        ////        len = strlen(p);
        len = cl.cmdi - (p - cl.cmd);
        while (f_gets(cl.tmp, sizeof (cl.tmp), fp) != NULL) {
            tmpLen = strlen(cl.tmp) - 1;
            if (cl.tmp[tmpLen] == '\n')
                cl.tmp[tmpLen] = '\0';
            if (tmpLen >= len && strncmp(p, cl.tmp, len) == 0) {
                if (cl.tmp[len] == '\0') {
                    occ = 0;
                    found = FALSE;
                    break;
                }
                if (occ == 0) {
                    match = cl.tmp[len];
                    occ++;
                    found = TRUE;
                } else {
                    if (match != cl.tmp[len]) {
                        found = FALSE;
                    }
                }
            }
        }

        if (found && occ > 0)
            CliAddCharAndUpdateConsole(match);

        put_rc(f_lseek(fp, 0l));
    } while (found && occ != 0);

    if (occ != 0) {
        while (f_gets(cl.tmp, sizeof (cl.tmp), fp) != NULL) {
            if (strncmp(p, cl.tmp, len) == 0) {
                tmpLen = strlen(cl.tmp) - 1;
                if (cl.tmp[tmpLen] == '\n')
                    cl.tmp[tmpLen] = '\0';
                printf("\r\n%s", cl.tmp);
            }
        }
        printf("\r\n");
        CliReprintConsole();
    }

    put_rc(f_close(fp));

    return occ;
}

int CliSearchCommand(char *search) {

    int i, len;
    char *pcmd;

    // Check between all the commands which match the string passed
    for (i = 0; i < NUMBER_OF_COMMANDS; i++) {
        // Create a char pointer to the name of the command
        pcmd = commands[i].name;
        // Save in local variable the lenght of the name command
        len = cmd_iterator[i].cmd_len;
        // Check that the string passed as long as the command is to be verified
        // Then, check the special termination character: ' '
        // And if it matches compares strings
        if ((search[len] == '\0' || search[len] == ' ') && strncmp(pcmd, search, len) == 0)
            return i;
    }

    // Return -1 if the string doesn't match any commands
    return -1;
}

void CliClearCommand() {
    //cl.cmd[0] = '\0';
    cl.cmdlen = cl.cmdi = 0;
}

void CliReprintConsole() {
    CliPrintEscape(escape_arrow_left, cl.cmdlen - cl.cmdi);
    if (cl.cmdlen)
    printf(">%s", cl.cmd);
    else
        putc('>');
}

BOOL CliAddStringAndUpdateConsole(char *str) {
    while (*str != '\0')
        CliAddCharAndUpdateConsole(*str++);
}

void CliPrintEscape(const char *p, int i) {
    while (i--)
        printf((char*) p);
}

void CliPrintBackspace() {
    char c = 0x7F;
    putc(c);
}

void CliPrintFor(char *p, int i, int len) {
    // Stampa gli elementi che sono da i a len
    while (i < len)
        putc(p[i++]);
}

BOOL CliAddCharAndUpdateConsole(unsigned char c) {

    int i;

    switch (c) {

        case ESCAPE_ARROW_LEFT:
            // Put the cursor to left; 0x1B 0x5B 0x44
            if (cl.cmdi > 0) {
                cl.cmdi--;
                CliPrintEscape(escape_arrow_left, 1);
            }
            break;

        case ESCAPE_ARROW_RIGHT:
            // Put the cursor to right; 0x1B 0x5B 0x43
            if (cl.cmdi < cl.cmdlen) {
                cl.cmdi++;
                CliPrintEscape(escape_arrow_right, 1);
            }
            break;

        case ESCAPE_HOME:

            CliPrintEscape(escape_arrow_left, cl.cmdi);
            cl.cmdi = 0;
            break;

        case ESCAPE_END:

            CliPrintEscape(escape_arrow_right, cl.cmdlen - cl.cmdi);
            cl.cmdi = cl.cmdlen;
            break;

        case ESCAPE_DEL:

            if (cl.cmdi < cl.cmdlen) {
                for (i = cl.cmdi; i < cl.cmdlen; i++) {
                    cl.cmd[i] = cl.cmd[i + 1];
                }
                // Sposto a destra di uno e cancello
                CliPrintEscape(escape_arrow_right, 1);
                CliPrintBackspace();

                // Decrementa la lunghezza del buffer in accordo al DEL
                cl.cmdlen--;

                // Stampa gli elementi che sono da i a len
                CliPrintFor(cl.cmd, cl.cmdi, cl.cmdlen);

                // Sposto a destra di uno e cancello
                CliPrintEscape(escape_arrow_right, 1);
                CliPrintBackspace();

                // Torna il curso in dietro di len - i posizioni
                CliPrintEscape(escape_arrow_left, cl.cmdlen - cl.cmdi);
            }
            break;

        case 0x7F:

            // Rimuovd un elmento dal buffer
            if (cl.cmdi > 0) {
                cl.cmdi--;
                for (i = cl.cmdi; i < cl.cmdlen; i++) {
                    cl.cmd[i] = cl.cmd[i + 1];
                }
                CliPrintBackspace();
                cl.cmdlen--;

                // Stampa gli elementi che sono da i a len
                CliPrintFor(cl.cmd, cl.cmdi, cl.cmdlen);

                // Sposto a destra di uno e cancello
                CliPrintEscape(escape_arrow_right, 1);
                CliPrintBackspace();

                // Torna il curso in dietro di len - i posizioni
                CliPrintEscape(escape_arrow_left, cl.cmdlen - cl.cmdi);
            }

            break;

        case ESCAPE_ARROW_UP:

            if (cl.lastCmd < cl.nCmd) {
                cl.lastCmd++;
                CliGetLastCommandFromFile(cl.lastCmd);
                if (config.console.echo)
                printf("\r>%s", cl.cmd);
                CliPrintEscape(escape_clear_end_row, 1);
            }
            break;

        case ESCAPE_ARROW_DOWN:

            if (cl.lastCmd > 1) {
                cl.lastCmd--;
                CliGetLastCommandFromFile(cl.lastCmd);
                if (config.console.echo)
                printf("\r>%s", cl.cmd);
                CliPrintEscape(escape_clear_end_row, 1);
            } else if (cl.lastCmd == 1) {
                cl.cmd[0] = '\0';
                cl.lastCmd = cl.cmdi = cl.cmdlen = 0;
                if (config.console.echo)
                printf("\r>%s", cl.cmd);
                CliPrintEscape(escape_clear_end_row, 1);
            }
            break;

        default:
            // Other printable character
            if (cl.cmdlen < CLI_MAX_BUF_SIZE && cl.cmdi < CLI_MAX_BUF_SIZE) {

                // Sposta gli elementi all'interno del buffer
                if (cl.cmdi < cl.cmdlen) {
                    for (i = cl.cmdlen; i > cl.cmdi; i--)
                        cl.cmd[i] = cl.cmd[i - 1];
                }

                // Aggiunge il carattere ricevuto al buffer e lo stampa
                cl.cmd[cl.cmdi++] = c;
                cl.cmd[++cl.cmdlen] = '\0';
                if (config.console.echo)
                putc(c);

                // Stampa gli elementi che sono da i a len
                CliPrintFor(cl.cmd, cl.cmdi, cl.cmdlen);

                // Torna il curso in dietro di len - i posizioni
                CliPrintEscape(escape_arrow_left, cl.cmdlen - cl.cmdi);
            }
            break;
    }

    return FALSE;
}

BOOL CliCreateFileListOfCommands() {

    extern FIL ftmp1;
    FIL *fp = &ftmp1;
    int i;

    // Create a temporary file where will create a list of commands
    if (!put_rc(f_open(fp, temporaryFileCommands, FA_WRITE | FA_CREATE_ALWAYS))) {
        // Unable to create temporary file.
        return FALSE;
    }

    // Changes the properties of the temporary file to hide it
    if (!put_rc(f_chmod(temporaryFileCommands, AM_HID, AM_HID))) {
        return FALSE;
    }

    // Prints all commands
    for (i = 0; i < NUMBER_OF_COMMANDS; i++) {
        f_printf(fp, "%s\n", commands[i].name);
    }

    // Close the temporary file
    if (!put_rc(f_close(fp))) {
        // Unable to close the temporary file
        return FALSE;
    }
    return TRUE;
}

BOOL CliCreateFileListOfFilesEntry(void) {

    extern FILINFO finfo;
    extern FIL ftmp1;
    FIL *fp = &ftmp1;
    extern DIR dir;

    // Create a temporary file where will create a list of files
    if (!put_rc(f_open(fp, temporaryFileEntryList, FA_WRITE | FA_CREATE_ALWAYS))) {
        // Unable to create temporary file.
        return FALSE;
    }

    // Changes the properties of the temporary file to hide it
    put_rc(f_chmod(temporaryFileEntryList, AM_HID, AM_HID));

    // Flush the hidden property
    put_rc(f_sync(fp));

    // Get the current directory name
    put_rc(f_getcwd(cl.tmp, CLI_MAX_DIR_SIZE));

    // Open the directory
    if (put_rc(f_opendir(&dir, cl.tmp))) {

        while (TRUE) {
            // Read a directory item
            if (!put_rc(f_readdir(&dir, &finfo)))
                break;
            // Break on end of dir
            if (finfo.fname[0] == 0)
                break;
            // Ignore dot entry
            if (finfo.fname[0] == '.')
                continue;

            if (!(finfo.fattrib & AM_HID)) {
                // It is a file
                f_printf(fp, "%s\n", GetFileNamePointer(&finfo));
            }
        }
        put_rc(f_closedir(&dir));
    }

    // Try to close temporary file
    if (!put_rc(f_close(fp))) {
        // Unable to close the temporary file
        return FALSE;
    }
    return TRUE;
}

void CliPutLastCommandInFile() {

    int i;
    int writed;

    // Place the file pointer at the end of the file
    put_rc(f_lseek(&fileLastCommands, f_size(&fileLastCommands)));

    for (i = cl.cmdlen; i < CLI_MAX_BUF_SIZE; i++)
        cl.cmd[i] = '\0';

    // Write into the file last command
    put_rc(f_write(&fileLastCommands, cl.cmd, CLI_MAX_BUF_SIZE, &writed));

    // Syncronize the content of the file on the micro SD
    put_rc(f_sync(&fileLastCommands));

    cl.nCmd++;
    cl.lastCmd = 0;
}

void CliGetLastCommandFromFile(int pos) {

    int read;

    // Place the file pointer at the end of the file
    put_rc(f_lseek(&fileLastCommands, f_size(&fileLastCommands) - (CLI_MAX_BUF_SIZE * pos)));

    // read last command
    put_rc(f_read(&fileLastCommands, cl.cmd, CLI_MAX_BUF_SIZE, &read));

    // Clear all indicator
    cl.cmdlen = cl.cmdi = strlen(cl.cmd);
}

void CliTooFewArgumnets(char *name) {
    printf("Too few arguments passed to function, type: help %s\r\n", name);
}

void CliTooManyArgumnets(char *name) {
    printf("Too many arguments passed to function, type: help %s\r\n", name);
}

void CliCommandNotFound(char *name) {
    printf("Command '%s' not found!\r\n", name);
}

void CliErrorOccurred(char *name) {
    printf("An error occured during the operation: '%s';\r\n", name);
}

void CliNoSuchFileOrDirectory(void) {
    verbosePrintf(VER_MIN, "\r\nNo such file or directory");
    GpioUpdateOutputState(GPIO_BIT_FILE_NOT_FOUND);
}

void CliFileOrDirectoryAlreadyExists(void) {
    printf("File or directory already exists\r\n");
}

void CliOnlyTextual() {
    printf("Only textual file are supported!\r\n");
}

void CliUsageExample(const char *str) {
    printf("\r\nExample: %s\r\n", str);
}

void CliUsageCommand(const char *str) {
    printf("\r\nUsage: %s\r\n", str);
}

void CliPrintListOfAllCommands(void) {

    int i;

    for (i = 0; i < NUMBER_OF_COMMANDS; i++)
        printf("%s\r\n", commands[i].name);
}