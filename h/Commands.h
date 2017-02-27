/*********************************************************************
 *
 *  Command Line Interpreter
 *
 *********************************************************************
 * FileName:        Commands.h
 * Dependencies:    CommandLineInterpreter.h
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
 * 1.0   Initial release 20 gennaio 2014, 19.20
 *
 ********************************************************************/

#ifndef _COMMANDS_H
#define	_COMMANDS_H

#include "GenericTypeDefs.h"
#include "FatFS/ff.h"


void Help(int, char**);
void List(int, char **);
void FileSystem(int, char **);
void Move(int, char **);
void Delete(int, char **);
void DeleteDir(int, char **);
void Clear(int, char **);
void ChangeDir(int, char **);
void Mkdir(int, char **);
void Cat(int, char **);
void Copy(int, char **);
void Reboot(int, char **);
void Debug(int, char **);
void Flash(int, char **);
void Version(int, char **);

BOOL CheckFileExistence(char *p);
const char *ByteToFatAttributes(BYTE);
char *GetFileNamePointer(FILINFO *);
BOOL put_rc(FRESULT);
BOOL checkFatAttributes(FILINFO *, BYTE);
FRESULT empty_directory(char* path);

FRESULT scan_files(char* path);
void put_dump(const BYTE *buff, DWORD ofs, BYTE cnt);

#endif	/* _COMMANDS_H */

