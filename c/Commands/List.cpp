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
#include "Utilities/printer.h"
#include "Utilities/Utilities.h"

List::List(CLI *cli) : CommandBase() {
    this->cli = cli;
    calculateNameLength();
}

const char * List::getCommandOptions(void) {
    return options;
}

const char * List::getCommandName(void) {
    return name;
}

int List::command(void) {

    int rtn;
    long size, integer, decimal;
    double sized;
    char units;
    FAT_TIME fat_time;

    rtn = COMMAND_BASE_EXECUTING;
    switch (sm) {
        case SM_LIST_HOME:
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
            sm = SM_LIST_INITIALIZE;
            //break; // no break required

        case SM_LIST_INITIALIZE:
// Reset file counter
            fres = FR_OK;
            countFile = countDir = countTotObj = totalSize = 0;

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

            // Allocate enough space for buf, DIR, and FINFO structures
            buf = NULL;
            buf = (char*) custom_malloc((void*) buf, LIST_BUFFER_SIZE);
            dir = NULL;
            dir = (DIR*) custom_malloc((void*) dir, sizeof (DIR));
            finfo = NULL;
            finfo = (FILINFO*) custom_malloc((void*) finfo, sizeof (FILINFO));
            // Copy the name of the selected working directory 
            if (path != NULL)
                strncpy(buf, path, LIST_BUFFER_SIZE);
            else
                fres = f_getcwd(buf, LIST_BUFFER_SIZE);

            sm = (fres == FR_OK) ? SM_LIST_OPEN_DIR : SM_LIST_ERROR;
            break;

        case SM_LIST_OPEN_DIR:
            // Open the given directory by name
            if ((fres = f_opendir(dir, buf))== FR_NO_PATH){
                cli->verbosePrintfWrapper(VER_ERR, true, "\r\nPath not found: %s", buf);
                cli->verbosePrintfWrapper(VER_ERR, true, "\r\nFatFs error: %s", string_rc(fres));
                sm = SM_LIST_ERROR;
            } else if  (fres == FR_OK)  {
                sm =SM_LIST_READ_DIR;
            } else {
                cli->verbosePrintfWrapper(VER_ERR, true, "\r\nFatFs error: %s", string_rc(fres));
                sm = SM_LIST_ERROR;
            }
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
                    if (flags.hidden == true || !(finfo->fattrib & AM_HID)) {
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

                        printf("%s %4d.%-2d %cByte  %d/%d/%d\t%02d:%02d:%02d\t%s\r\n", byteToFatAttributes(finfo->fattrib), integer, decimal, units, (fat_time.fields.year + 1980), fat_time.fields.month, fat_time.fields.day, fat_time.fields.hour, fat_time.fields.min, (fat_time.fields.sec * 2), finfo->fname);

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
                        printf("%s\r\n", finfo->fname);
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
                    printf("Total item(s): %d\r\n", countTotObj);

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
            fres = f_closedir(dir);
            ////put_rc(fres);
            sm = SM_LIST_END;
            break;

        case SM_LIST_END:
            // Release allocated memory
            if (finfo != NULL)
                custom_free((void**) &finfo);
            if (dir != NULL)
                custom_free((void**) &dir);
            if (buf != NULL)
                custom_free((void**) &buf);
            sm = SM_LIST_HOME;
            rtn = COMMAND_BASE_TERMINATED;
            break;
    }

    return rtn;
}

const char * List::byteToFatAttributes(uint8_t att) {

    static char str[10];
    str[0] = '\0';

    //#define	AM_RDO	0x01	/* Read only */
    //#define	AM_HID	0x02	/* Hidden */
    //#define	AM_SYS	0x04	/* System */
    //#define	AM_VOL	0x08	/* Volume label */
    //#define   AM_LFN	0x0F	/* LFN entry */
    //#define   AM_DIR	0x10	/* Directory */
    //#define   AM_ARC	0x20	/* Archive */
    //#define   AM_MASK	0x3F	/* Mask of defined bits */

    strcat(str, ((att & AM_HID) == AM_HID) ? "h" : "-");
    //    strcat(str, ((att & AM_SYS) == AM_SYS) ? "s" : "-");
    //    strcat(str, ((att & AM_VOL) == AM_VOL) ? "v" : "-");
    //    strcat(str, ((att & AM_LFN) == AM_LFN) ? "l" : "-");
    strcat(str, ((att & AM_DIR) == AM_DIR) ? "d" : "-");
    //    strcat(str, ((att & AM_ARC) == AM_ARC) ? "a" : "-");
    strcat(str, ((att & AM_RDO) == AM_RDO) ? "r-" : "rw");

    return str;
}