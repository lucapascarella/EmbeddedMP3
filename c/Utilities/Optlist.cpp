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

#include "Utilities/Optlist.hpp"
#include "Utilities/CustomFunctions.h"

Optlist::Optlist(void) {
    argc = 0;
}

return_t *Optlist::createOptionList(int argc, char * argv[], const char *options) {
    return_t *rtn;

    int nextArg;
    option_t *optHead, *optTail;
    int optIndex, argIndex;
    argument_t *newArgument, **ptrToNextArg;
    //int optionsLen;
    argument_t *argHead, *argTail;

    // Start with first argument and nothing found
    nextArg = 1;
    optHead = NULL;
    optTail = NULL;

    argHead = NULL;
    argTail = NULL;

    // Prepare the return_t structure with optHead and argHead
    rtn = (return_t*) custom_malloc(rtn, sizeof (return_t));
    rtn->opt = NULL;
    rtn->arg = NULL;

    // Loop through all of the command line arguments
    while (nextArg < argc) {
        argIndex = 1;
        while ((custom_strlen(argv[nextArg]) > argIndex) && ('-' == argv[nextArg][0])) {
            optIndex = MatchOpt(argv[nextArg][argIndex], options);
            if (options[optIndex] == argv[nextArg][argIndex]) {
                //if (optIndex < optionsLen) {
                // We found the matching option
                if (NULL == optHead) {
                    optHead = MakeOpt(options[optIndex], NULL, OL_NOINDEX);
                    rtn->opt = optHead;
                    optTail = optHead;
                } else {
                    optTail->next = MakeOpt(options[optIndex], NULL, OL_NOINDEX);
                    optTail = optTail->next;
                }

                //		if (':' == options[optIndex + 1]) {
                //		    // The option found should have a text arguement
                //		    argIndex++;
                //
                //		    // Create a new object of argument_t and initialize it to NULL
                //		    newArgument = MakeArg();
                //
                //		    if (strlen(argv[nextArg]) > argIndex) {
                //			// No space between argument and option
                //			////tail->argument = &(argv[nextArg][argIndex]);
                //			////tail->argIndex = nextArg;
                //			newArgument->argument = &(argv[nextArg][argIndex]);
                //		    } else if (nextArg < argc) {
                //			// There must be space between the argument option
                //			nextArg++;
                //			////tail->argument = argv[nextArg];
                //			////tail->argIndex = nextArg;
                //			newArgument->argument = argv[nextArg];
                //		    }
                //		    newArgument->argIndex = nextArg;
                //
                //		    tail->nextArgument = newArgument;
                //		    tail->argNumber = 1;
                //		    // Done with argv[nextArg]
                //		    break;
                //		} else
                if (';' == options[optIndex + 1] || ':' == options[optIndex + 1]) {
                    // The option found can have one or more text arguements

                    argIndex++;
                    //thisArg = nextArg;
                    while ((nextArg + 1) <= argc || '-' != argv[nextArg + 1][0]) {

                        // Create a new object of argument_t and initialize it to NULL
                        newArgument = MakeArg();

                        if (custom_strlen(argv[nextArg]) > argIndex) {
                            // No space between argument and option
                            ////tail->argument = &(argv[nextArg][argIndex]);
                            ////tail->argIndex = nextArg;
                            newArgument->argument = &(argv[nextArg][argIndex]);
                            argIndex = custom_strlen(argv[nextArg]);
                        } else if ((nextArg + 1) < argc) {
                            // There must be space between the argument option
                            nextArg++;
                            ////tail->argument = argv[nextArg];
                            ////tail->argIndex = nextArg;
                            newArgument->argument = argv[nextArg];
                        } else {
                            FreeArgList(newArgument);
                            break;
                        }
                        newArgument->argIndex = nextArg;

                        argIndex = custom_strlen(argv[nextArg]);

                        // Appends the new Object to last position in the queue
                        ptrToNextArg = &optTail->nextArgument;
                        while (*ptrToNextArg != NULL)
                            ptrToNextArg = &((*ptrToNextArg)->nextArgument);
                        *ptrToNextArg = newArgument;
                        optTail->argNumber++;
                        if (':' == options[optIndex + 1] || (nextArg + 1) >= argc) {
                            argIndex++;
                            break;
                        }
                    }
                    // Done with argv[nextArg]
                    break;
                }
            }
            argIndex++;
        }
        //
        if ((custom_strlen(argv[nextArg]) >= argIndex) && ('-' != argv[nextArg][0])) {
            // Parse a non option arguments
            if (NULL == argHead) {
                argHead = MakeArg(); //MakeOpt(options[optIndex], NULL, OL_NOINDEX);
                rtn->arg = argHead;
                argHead->argument = argv[nextArg];
                argHead->argIndex = nextArg;
                argHead->nextArgument = NULL;
                argTail = argHead;
            } else {
                argTail->nextArgument = MakeArg(); //MakeOpt(options[optIndex], NULL, OL_NOINDEX);
                argTail->nextArgument->argument = argv[nextArg];
                argTail->nextArgument->argIndex = nextArg;
                argTail->nextArgument->nextArgument = NULL;
                argTail = argTail->nextArgument;
            }
        }
        //
        nextArg++;
    }

    return rtn;
}

argument_t *Optlist::MakeArg(void) {
    argument_t *newArgument;

    // Create a new object of argument_t and initialize it to NULL
    newArgument = (argument_t*) custom_malloc(newArgument, sizeof (argument_t));

    if (newArgument != NULL) {
        newArgument->nextArgument = NULL;
        newArgument->argument = NULL;
        newArgument->argIndex = 0;
    } else {
        ////verbosePrintf(VER_DBG, "Failed to Allocate argument_t");
    }

    return newArgument;
}

option_t *Optlist::MakeOpt(const char option, char *const argument, const int index) {
    option_t *opt;

    opt = (option_t*) custom_malloc(opt, sizeof (option_t));

    if (opt != NULL) {
        opt->option = option;
        ////opt->argument = argument;
        ////opt->argIndex = index;
        opt->next = NULL;
        // Mod Luca
        opt->argNumber = 0;
        opt->nextArgument = NULL;
        // End Mod Luca
    } else {
        //// verbosePrintf(VER_DBG, "Failed to Allocate option_t");
    }

    return opt;
}

void Optlist::FreeArgList(argument_t *list) {
    argument_t *head, *next;

    head = list;
    list = NULL;

    while (head != NULL) {
        next = head->nextArgument;
        free(head);
        head = next;
    }
}

int Optlist::MatchOpt(const char argument, const char * options) {
    int optIndex = 0;

    // Attempt to find a matching option
    while ((options[optIndex] != '\0') && (options[optIndex] != argument)) {
        do {
            optIndex++;
        } while ((options[optIndex] != '\0') && ((':' == options[optIndex]) || (';' == options[optIndex])));
    }

    return optIndex;
}


char * Optlist::getArgumentFromOption(char option) {

}

Optlist::~Optlist(void) {

}
