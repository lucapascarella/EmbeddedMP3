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

#include "Commands/List.hpp"
#include "Utilities/ArgsParser.hpp"
#include "Utilities/CustomFunctions.h"
#include "Utilities/RTCC.h"

List::List(void) : CommandBase() {
    calculateNameLength();
    commandState = COMMAND_STATE_HOME;
}

const char * List::getCommandOptions(void) {
    return options;
}

const char * List::getCommandName(void) {
    return name;
}

int List::command(void) {

    int rtn;

    switch (commandState) {
        case COMMAND_STATE_HOME:
            commandState = COMMAND_STATE_PARSE_ARGS;
            //break: // no break needed

        case COMMAND_STATE_PARSE_ARGS:
            // Check flag options
            path = NULL;
            flags.human = flags.list = flags.hidden = false;
            if (opt->isOptionPresent('a'))
                flags.hidden = true;
            if (opt->isOptionPresent('l'))
                flags.list = true;
            if (opt->isOptionPresent('h'))
                flags.human = true;
            if (opt->isOptionPresent('&'))
                path = opt->getFirstArgumentForOption('&');
            // Continue with command
            rtn = COMMAND_BASE_EXECUTING;
            commandState = COMMAND_STATE_EXECUTE;
            //break; // no break required

        case COMMAND_STATE_EXECUTE:
            if ((rtn = executeCommandBody()) != COMMAND_BASE_EXECUTING)
                commandState = COMMAND_STATE_DONE;
            break;

        case COMMAND_STATE_DONE:
            commandState = COMMAND_STATE_HOME;
            break;
    }

    return rtn;
}

int List::executeCommandBody(void) {

    long size, integer, decimal;
    double sized;
    char units;
    int rtn;
    FAT_TIME fat_time;

    rtn = COMMAND_BASE_EXECUTING;
    switch (sm) {
        case SM_LIST_HOME:
            if (path != NULL)
                strncpy(buf, path, LIST_BUFFER_SIZE);
            else
                fres = f_getcwd(buf, LIST_BUFFER_SIZE);



            //            fres = FR_OK;
            //            size = strlen(argv[argc - 1]);
            //            // Delete the very last '/' to avoid the FatFS FR_INVALID_NAME error
            //            if (size != 1 && argv[argc - 1][size - 1] == '/')
            //                argv[argc - 1][size - 1] = '\0';
            //            if (flags.relative)
            //                strncpy(buf, argv[argc - 1], LIST_BUFFER_SIZE);
            //            else
            //                fres = f_getcwd(buf, LIST_BUFFER_SIZE);
            //            dir = &dir;
            //            finfo = &finfo;
            // Allocate enough space for DIR and FINFO structures
            dir = NULL;
            dir = (DIR*) custom_malloc((void*) dir, sizeof (DIR));
            finfo = NULL;
            finfo = (FILINFO*) custom_malloc((void*) finfo, sizeof (FILINFO));

            sm = (fres == FR_OK) ? SM_LIST_OPEN_DIR : SM_LIST_ERROR;
            break;

        case SM_LIST_OPEN_DIR:
            // Open the given directory name
            fres = f_opendir(dir, buf);
            // Reset file counter
            countFile = countDir = countTotObj = totalSize = 0;
            sm = (fres == FR_OK) ? SM_LIST_READ_DIR : (fres == FR_NO_PATH) ? SM_LIST_NO_PATH : SM_LIST_ERROR;
            break;

        case SM_LIST_READ_DIR:
            // Read contents
            fres = f_readdir(dir, finfo);
            if (fres == FR_OK) {
                // Break on end of dir
                if (finfo->fname[0] == 0) {
                    // Current directory is empty
                    sm = SM_LIST_CLOSE_DIR;
                } else if (flags.list) {
                    if (flags.hidden == TRUE || !(finfo->fattrib & AM_HID)) {
                        size = finfo->fsize;
                        if (size < 1024) {
                            sized = size;
                            units = ' ';
                        } else if (size < 1048576) {
                            sized = (double) (size / 1024.0);
                            units = 'k';
                        } else if (size < 1073741824) {
                            sized = size / 1048576.0;
                            units = 'M';
                        } else {
                            sized = size / 1073741824.0;
                            units = 'G';
                        }
                        integer = sized;
                        decimal = (sized - integer) * 100;

                        fat_time.word.data = finfo->fdate;
                        fat_time.word.time = finfo->ftime;

                        ////printf("%s %4d.%-2d %cByte  %d/%d/%d\t%02d:%02d:%02d\t%s\r\n", ByteToFatAttributes(finfo->fattrib), integer, decimal, units, (fat_time.fields.year + 1980), fat_time.fields.month, fat_time.fields.day, fat_time.fields.hour, fat_time.fields.min, (fat_time.fields.sec * 2), GetFileNamePointer(finfo));

                        countTotObj++;
                        if (finfo->fattrib & AM_DIR) {
                            countDir++;
                        } else {
                            countFile++;
                            totalSize += finfo->fsize;
                        }
                    }
                } else {
                    // Skip dot entry
                    if ((!(finfo->fattrib & AM_HID) || flags.hidden) && finfo->fname[0] != '.') {
                        // It is a file or directory
                        ////printf("%s\r\n", GetFileNamePointer(finfo));
                    }
                }
            } else {
                sm = SM_LIST_ERROR;
            }
            break;

        case SM_LIST_CLOSE_DIR:
            // Close the current directory
            fres = f_closedir(dir);

            if (flags.list) {
                if (countTotObj == 0)
                    printf("The directory is empty\r\n");
                else
                    printf("Total itme(s): %d\r\n", countTotObj);

                printf("%4u File(s)%8lu kBytes\r\n%4u Dir(s)\r\n", countFile, totalSize / 1024, countDir);
                //                if (f_getfree("0", (DWORD*) & totalSize, &fs) == FR_OK)
                //                    printf(", %10lu Mbytes free\r\n", totalSize * fs->csize / 2 / 1024);
            }
            sm = (fres == FR_OK) ? SM_LIST_END : SM_LIST_ERROR;
            break;

        case SM_LIST_NO_PATH:
            printf("Not found\r\n");
            sm = SM_LIST_END;
            break;

        case SM_LIST_ERROR:
            ////put_rc(fres);
            sm = SM_LIST_END;
            break;

        case SM_LIST_END:
            sm = SM_LIST_HOME;
            rtn = 0;
            break;
    }

    return rtn;
}