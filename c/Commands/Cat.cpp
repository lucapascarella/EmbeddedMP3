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

#include "Commands/Cat.hpp"
#include "Utilities/ArgsParser.hpp"
#include "Utilities/CustomFunctions.h"
//#include "Utilities/RTCC.h"
#include "Utilities/printer.h"
#include "Utilities/Utilities.h"
#include "CLI.hpp"
#include <ctype.h>

Cat::Cat(CLI *cli) : CommandBase(cli) {
    //calculateNameLength();
}

const char * Cat::getCommandOptions(void) {
    return options;
}

const char * Cat::getCommandName(void) {
    return name;
}

int Cat::command(void) {

    int rtn, i;
    //FAT_TIME fat_time;
    char *p;

    rtn = COMMAND_BASE_EXECUTING;
    switch (sm) {
        case SM_CAT_HOME:
            // Check flag options
            path = NULL;
            flags.binary = false;
            if (opt->isOptionPresent('b'))
                flags.binary = true;
            if (opt->isOptionPresent('&'))
                path = opt->getFirstArgumentForOption('&');
            // Continue with command
            rtn = COMMAND_BASE_EXECUTING;
            sm = SM_CAT_INITIALIZE;
            //break; // no break required

        case SM_CAT_INITIALIZE:
            // Reset file counter
            fres = FR_OK;
            // Allocate enough space for buf, FIL, and FINFO structures
            buf = NULL;
            buf = (char*) custom_malloc((void*) buf, CAT_BUFFER_SIZE);
            fil = NULL;
            fil = (FIL*) custom_malloc((void*) fil, sizeof (FIL));
            finfo = NULL;
            finfo = (FILINFO*) custom_malloc((void*) finfo, sizeof (FILINFO));
            // Copy the name of the selected working directory 
            if (path != NULL)
                strncpy(buf, path, CAT_BUFFER_SIZE);
            else
                fres = f_getcwd(buf, CAT_BUFFER_SIZE);

            sm = (fres == FR_OK) ? SM_CAT_OPEN_FILE : SM_CAT_ERROR;
            break;

        case SM_CAT_CHECK_FILE:
            if (flags.binary || (p = strrchr(path, '.')) != NULL) {
                // Copy the extensions in local buffer
                while ((buf[i++] = toupper(*p++)) != '\0' && i < CAT_BUFFER_SIZE);
                buf[CAT_BUFFER_SIZE - 1] = '\0';
                if (flags.binary || strstr(buf, "TXT") != NULL || strstr(buf, "INI") != NULL || strstr(buf, "PLS") != NULL) {
                    // Test file existence
                    fres = f_stat(path, finfo);
                    if (fres == FR_NO_FILE) {
                        printf("File does not exist\r\n");
                    }
                    sm = (fres == FR_OK) ? SM_CAT_OPEN_FILE : SM_CAT_ERROR;
                } else {
                    ////CliOnlyTextual();
                    sm = SM_CAT_END;
                }
            } else {
                ////CliOnlyTextual();
                sm = SM_CAT_END;
            }
            break;

        case SM_CAT_OPEN_FILE:
            // Open given file
            fres = f_open(fil, argv[1], FA_READ);
            sm = (fres == FR_OK) ? SM_CAT_READ_FILE : SM_CAT_ERROR;
            printf("Size: %u byte\r\n", finfo->fsize);
            break;

        case SM_CAT_READ_FILE:
            break;

        case SM_CAT_PRINT:
            break;


        case SM_CAT_CLOSE_FILE:
            break;

        case SM_CAT_ERROR:
            sm = SM_CAT_END;
            break;

        case SM_CAT_END:
            // Release allocated memory
            if (finfo != NULL)
                custom_free((void**) &finfo);
            if (buf != NULL)
                custom_free((void**) &buf);
            sm = SM_CAT_HOME;
            rtn = COMMAND_BASE_TERMINATED;
            break;
    }
    return rtn;
}

int Cat::helper(void) {
    // Command cat
    printf("Print the contents of a textual file.\r\n");
    this->usageCommand("cat file [-bin]");
    printf("file\tName of textual file (.txt, .ini or .pls)\r\n");
    printf("-bin\tforce binary transfer\r\n");
    this->usageExample("cat conf.ini");
    return COMMAND_BASE_TERMINATED;
}