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

#include "Utilities/ArgsParser.hpp"
#include "Utilities/Utilities.h"

ArgsParser::ArgsParser(void) {
    argc = 0;
    argv[0] = 0;
    cmdLineCopy = NULL;
}

int ArgsParser::extractArgs(char *input) {

    int length;

    // Set to no argv elements, in case we have to bail out
    argv[0] = 0;
    argc = 0;

    // Allocate space for a manipulated copy of input
    length = custom_strlen(input);
    if (cmdLineCopy != NULL)
        custom_free((void**) &cmdLineCopy);
    cmdLineCopy = (char*) custom_malloc(cmdLineCopy, length);
    custom_memset(cmdLineCopy, '\0', length);

    if ('"' == *cmdLineCopy) {
        // If command line starts with a quote ("), it's a quoted filename.  Skip to next quote.
        cmdLineCopy++;
        // argv[0] == executable name
        argv[0] = cmdLineCopy;
        while (*cmdLineCopy && (*cmdLineCopy != '"'))
            cmdLineCopy++;

        if (*cmdLineCopy) // Did we see a non-NULL ending?
            *cmdLineCopy++ = '\0'; // Null terminate and advance to next char
        else
            return 0; // Oops!  We didn't see the end quote
    } else {
        // A regular (non-quoted) filename
        // argv[0] == executable name
        argv[0] = cmdLineCopy;

        while (*cmdLineCopy && (' ' != *cmdLineCopy) && ('\t' != *cmdLineCopy))
            cmdLineCopy++;

        if (*cmdLineCopy)
            *cmdLineCopy++ = '0'; // Null terminate and advance to next char
    }

    // Done processing argv[0] (i.e., the executable name).  Now do the actual arguments
    argc = 1;
    while (true) {
        // Skip over any whitespace
        while (*cmdLineCopy && ((' ' == *cmdLineCopy) || ('\t' == *cmdLineCopy)))
            cmdLineCopy++;

        if (0 == *cmdLineCopy) // End of command line???
            return argc;

        if ('"' == *cmdLineCopy) {
            // Argument starting with a quote???
            cmdLineCopy++; // Advance past quote character

            argv[argc++] = cmdLineCopy;
            argv[argc] = 0;

            // Scan to end quote, or NULL terminator
            while (*cmdLineCopy && (*cmdLineCopy != '"'))
                cmdLineCopy++;

            if (0 == *cmdLineCopy)
                return argc;

            if (*cmdLineCopy)
                *cmdLineCopy++ = 0; // Null terminate and advance to next char
        } else {
            // Non-quoted argument
            argv[argc++] = cmdLineCopy;
            argv[argc] = 0;

            // Skip till whitespace or NULL terminator
            while (*cmdLineCopy && (' ' != *cmdLineCopy) && ('\t' != *cmdLineCopy))
                cmdLineCopy++;

            if (0 == *cmdLineCopy)
                return argc;

            if (*cmdLineCopy)
                *cmdLineCopy++ = 0; // Null terminate and advance to next char
        }

        if (argc >= (_MAX_CMD_LINE_ARGS))
            return argc;
    }
}

char * ArgsParser::getArgPointer(int pos) {
    if (pos >= 0 && pos < argc) {
        return argv[pos];
    }
    return NULL;
}

int ArgsParser::getArgc(void) {
    return argc;
}

char **ArgsParser::getArgv(void) {
    return argv;
}

char * ArgsParser::getLastArgPointer(void) {
    return this->getArgPointer(argc - 1);
}

ArgsParser::~ArgsParser(void) {
    custom_free((void**) &cmdLineCopy);
}
