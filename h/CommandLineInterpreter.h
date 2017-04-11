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

#ifndef _COMMANDLINEINTERPRETER_H
#define	_COMMANDLINEINTERPRETER_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "MP3/MP3.h"
#include "FatFS/ff.h"

#define CLI_MODE                    0

    typedef struct {
        char *name;
        int (*cmd)(int, char**);
    } _command;

    typedef struct {
        BOOL cmd_match;
        int cmd_len;
    } _command_iterator;

#define CLI_MAX_DIR_SIZE    128
#define CLI_MAX_BUF_SIZE    1024 * 4

    typedef struct {
        char cmd[CLI_MAX_BUF_SIZE];
        int cmdi;
        int cmdlen;

        char tmp[CLI_MAX_BUF_SIZE];
        int tmpi;
        int tmplen;

        int lastCmd;
        int nCmd;

    } _command_line;


#define ESCAPE                  0x1B
#define ESCAPE_ARROW_LEFT       0x80
#define ESCAPE_ARROW_RIGHT      0x81
#define ESCAPE_ARROW_UP         0x82
#define ESCAPE_ARROW_DOWN       0x83

#define ESCAPE_PAGE_UP          0x84
#define ESCAPE_PAGE_DOWN        0x85
#define ESCAPE_HOME             0x86
#define ESCAPE_END              0x87

#define ESCAPE_DEL              0x88

#define ESCAPE_F1               0x89


    BOOL InitCli(void);
    void CliHandler(void);
    BOOL CliGetCharFromConsole(char *);
    BYTE CliCompleteCommand(void);
    BYTE CliCompleteCommandSearchInFile(char *, char *);
    int CliSearchCommand(char *);

    void CliClearCommand(void);
    void CliReprintConsole(void);
    BOOL CliAddStringAndUpdateConsole(char *);
    void CliPrintEscape(const char *p, int i);
    void CliPrintBackspace(void);
    void CliPrintFor(char *, int, int);
    BOOL CliAddCharAndUpdateConsole(unsigned char);
    BOOL CliCreateFileListOfCommands(void);
    BOOL CliCreateFileListOfFilesEntry(void);
    void CliPutLastCommandInFile(void);
    void CliGetLastCommandFromFile(int);

    void CliTooFewArgumnets(char *);
    void CliTooManyArgumnets(char *);
    void CliCommandNotFound(char *);
    void CliErrorOccurred(char *name);
    void CliNoSuchFileOrDirectory(void);
    void CliFileOrDirectoryAlreadyExists(void);
    void CliOnlyTextual(void);
    void CliUsageExample(const char *str);
    void CliUsageCommand(const char *str);
    void CliPrintListOfAllCommands(void);

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif	/* _COMMANDLINEINTERPRETER_H */

