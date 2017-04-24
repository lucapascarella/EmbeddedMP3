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
#include "Utilities/CustomFunctions.h"

ArgsParser::ArgsParser(void) {
    argc = 0;
    argv[0] = '\0';
    cmdLinePointer = NULL;
}

int ArgsParser::extractArgs(uint8_t *input) {

    int length;
    char *cmdLine;

    // Allocate space for a manipulated copy of input + '\0' terminator
    length = custom_strlen((char*) input) + 1;
    cmdLinePointer = (char*) custom_malloc(cmdLinePointer, length);
    custom_memcpy(cmdLinePointer, input, length);
    cmdLine = cmdLinePointer;

    if ('"' == *cmdLine) {
        // If command line starts with a quote ("), it's a quoted filename.  Skip to next quote.
        cmdLine++;
        // argv[0] == executable name
        argv[0] = cmdLine;
        while (*cmdLine && (*cmdLine != '"'))
            cmdLine++;

        if (*cmdLine) // Did we see a non-NULL ending?
            *cmdLine++ = '\0'; // Null terminate and advance to next char
        else
            return 0; // Oops!  We didn't see the end quote
    } else {
        // A regular (non-quoted) filename
        // argv[0] == executable name
        argv[0] = cmdLine;

        while (*cmdLine && (' ' != *cmdLine) && ('\t' != *cmdLine))
            cmdLine++;

        if (*cmdLine)
            *cmdLine++ = '\0'; // Null terminate and advance to next char
    }

    // Done processing argv[0] (i.e., the executable name).  Now do the actual arguments
    argc = 1;
    while (true) {
        // Skip over any whitespace
        while (*cmdLine && ((' ' == *cmdLine) || ('\t' == *cmdLine)))
            cmdLine++;

        if (0 == *cmdLine) // End of command line???
            return argc;

        if ('"' == *cmdLine) {
            // Argument starting with a quote???
            cmdLine++; // Advance past quote character

            argv[argc++] = cmdLine;
            argv[argc] = 0;

            // Scan to end quote, or NULL terminator
            while (*cmdLine && (*cmdLine != '"'))
                cmdLine++;

            if (0 == *cmdLine)
                return argc;

            if (*cmdLine)
                *cmdLine++ = 0; // Null terminate and advance to next char
        } else {
            // Non-quoted argument
            argv[argc++] = cmdLine;
            argv[argc] = 0;

            // Skip till whitespace or NULL terminator
            while (*cmdLine && (' ' != *cmdLine) && ('\t' != *cmdLine))
                cmdLine++;

            if (0 == *cmdLine)
                return argc;

            if (*cmdLine)
                *cmdLine++ = 0; // Null terminate and advance to next char
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
    if (cmdLinePointer != NULL)
        custom_free((void**) &cmdLinePointer);
}
