/***************************************************************************
 *                       Command Line Option Parser
 *
 *   File    : optlist.c
 *   Purpose : Provide getopt style command line option parsing
 *   Author  : Michael Dipperstein
 *   Date    : August 1, 2007
 *
 ****************************************************************************
 *   HISTORY
 *
 *   $Id: optlist.c,v 1.2 2008/12/25 05:33:28 michael Exp $
 *   $Log: optlist.c,v $
 *   Revision 1.2  2008/12/25 05:33:28  michael
 *   Add support for multiple options following a single dash ('-').
 *
 *   Revision 1.1.1.2  2007/09/04 04:45:42  michael
 *   Added FreeOptList.
 *
 *   Revision 1.1.1.1  2007/08/07 05:01:48  michael
 *   Initial Release
 *
 ****************************************************************************
 *
 * OptList: A command line option parsing library
 * Copyright (C) 2007 by Michael Dipperstein (mdipper@alumni.engr.ucsb.edu)
 *
 * This file is part of the OptList library.
 *
 * OptList is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * OptList is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/

/***************************************************************************
 *                             INCLUDED FILES
 ***************************************************************************/
#include "Utilities/Optlist.h"
#include "Utilities/printer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************************
 *                            TYPE DEFINITIONS
 ***************************************************************************/

/***************************************************************************
 *                                CONSTANTS
 ***************************************************************************/

/***************************************************************************
 *                            GLOBAL VARIABLES
 ***************************************************************************/

/***************************************************************************
 *                               PROTOTYPES
 ***************************************************************************/
argument_t *MakeArg(void);
option_t *MakeOpt(const char option, char *const argument, const int index);
int MatchOpt(const char argument, char *const options);

/***************************************************************************
 *                                FUNCTIONS
 ***************************************************************************/

/****************************************************************************
 *   Function   : GetOptList
 *   Description: This function is similar to the POSIX function getopt.  All
 *                options and their corresponding arguments are returned in a
 *                linked list.  This function should only be called once per
 *                an option list and it does not modify argv or argc.
 *   Parameters : argc - the number of command line arguments (including the
 *                       name of the executable)
 *                argv - pointer to the open binary file to write encoded
 *                       output
 *                options - getopt style option list.  A NULL terminated
 *                          string of single character options.  Follow an
 *                          option with a colon to indicate that it requires
 *                          an argument.
 *   Effects    : Creates a link list of command line options and their
 *                arguments.
 *   Returned   : option_t type value where the option and arguement fields
 *                contain the next option symbol and its argument (if any).
 *                The argument field will be set to NULL if the option is
 *                specified as having no arguments or no arguments are found.
 *                The option field will be set to PO_NO_OPT if no more
 *                options are found.
 *
 *   NOTE: The caller is responsible for freeing up the option list when it
 *         is no longer needed.
 ****************************************************************************/
return_t *GetOptList(const int argc, char *const argv[], char *const options) {

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
    rtn = malloc(sizeof (return_t));
    rtn->opt = NULL;
    rtn->arg = NULL;


    // Loop through all of the command line arguments
    while (nextArg < argc) {
        argIndex = 1;

        while ((strlen(argv[nextArg]) > argIndex) && ('-' == argv[nextArg][0])) {

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

                        if (strlen(argv[nextArg]) > argIndex) {
                            // No space between argument and option
                            ////tail->argument = &(argv[nextArg][argIndex]);
                            ////tail->argIndex = nextArg;
                            newArgument->argument = &(argv[nextArg][argIndex]);
                            argIndex = strlen(argv[nextArg]);
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

                        argIndex = strlen(argv[nextArg]);

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
        if ((strlen(argv[nextArg]) >= argIndex) && ('-' != argv[nextArg][0])) {
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

/****************************************************************************
 *   Function   : MakeArg
 *   Description: This function uses malloc to allocate space for an argument_t
 *                type structure and initailizes the structure to NULL
 *   Parameters : void
 *   Effects    : A new argument_t type variable is created on the heap.
 *   Returned   : Pointer to newly created and initialized argument_t type
 *                structure.  NULL if space for structure can't be allocated.
 ****************************************************************************/
argument_t *MakeArg(void) {
    argument_t *newArgument;

    // Create a new object of argument_t and initialize it to NULL
    newArgument = malloc(sizeof (argument_t));

    if (newArgument != NULL) {
        newArgument->nextArgument = NULL;
        newArgument->argument = NULL;
        newArgument->argIndex = 0;
    } else {
        verbosePrintf(VER_DBG, "Failed to Allocate argument_t");
    }

    return newArgument;
}

/****************************************************************************
 *   Function   : MakeOpt
 *   Description: This function uses malloc to allocate space for an option_t
 *                type structure and initailizes the structure with the
 *                values passed as a parameter.
 *   Parameters : option - this option character
 *                argument - pointer string containg the argument for option.
 *                           Use NULL for no argument
 *                index - argv[index] contains argument us OL_NOINDEX for
 *                        no argument
 *   Effects    : A new option_t type variable is created on the heap.
 *   Returned   : Pointer to newly created and initialized option_t type
 *                structure.  NULL if space for structure can't be allocated.
 ****************************************************************************/
option_t *MakeOpt(const char option, char *const argument, const int index) {
    option_t *opt;

    opt = malloc(sizeof (option_t));

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
        verbosePrintf(VER_DBG, "Failed to Allocate option_t");
    }

    return opt;
}

/****************************************************************************
 *   Function   : FreeRtnList
 *   Description: This function will free all the elements in an return_t
 *                type linked list starting from the node passed as a
 *                parameter.
 *   Parameters : list - head of linked list to be freed
 *   Effects    : All elements of the linked list pointed to by list will
 *                be freed and list will be set to NULL.
 *   Returned   : None
 ****************************************************************************/
void FreeRtnList(return_t *rtn) {

    FreeOptList(rtn->opt);
    FreeArgList(rtn->arg);
    free(rtn);
}

/****************************************************************************
 *   Function   : FreeOptList
 *   Description: This function will free all the elements in an option_t
 *                type linked list starting from the node passed as a
 *                parameter.
 *   Parameters : list - head of linked list to be freed
 *   Effects    : All elements of the linked list pointed to by list will
 *                be freed and list will be set to NULL.
 *   Returned   : None
 ****************************************************************************/
void FreeOptList(option_t *list) {
    option_t *head, *next;

    head = list;
    list = NULL;

    while (head != NULL) {
        next = head->next;
        // Mod Luca
        FreeArgList(head->nextArgument);
        // End Mod Luca
        free(head);
        head = next;
    }
}

/****************************************************************************
 *   Function   : FreeArgList
 *   Description: This function will free all the elements in an option_t
 *                type linked list starting from the node passed as a
 *                parameter.
 *   Parameters : list - head of linked list to be freed
 *   Effects    : All elements of the linked list pointed to by list will
 *                be freed and list will be set to NULL.
 *   Returned   : None
 ****************************************************************************/
void FreeArgList(argument_t *list) {
    argument_t *head, *next;

    head = list;
    list = NULL;

    while (head != NULL) {
        next = head->nextArgument;
        free(head);
        head = next;
    }
}

/****************************************************************************
 *   Function   : MatchOpt
 *   Description: This function searches for an arguement in an option list.
 *                It will return the index to the option matching the
 *                arguement or the index to the NULL if none is found.
 *   Parameters : arguement - character arguement to be matched to an
 *                            option in the option list
 *                options - getopt style option list.  A NULL terminated
 *                          string of single character options.  Follow an
 *                          option with a colon to indicate that it requires
 *                          an argument.
 *   Effects    : None
 *   Returned   : Index of argument in option list.  Index of end of string
 *                if arguement does not appear in the option list.
 ****************************************************************************/
int MatchOpt(const char argument, char *const options) {
    int optIndex = 0;

    // Attempt to find a matching option
    while ((options[optIndex] != '\0') && (options[optIndex] != argument)) {
        do {
            optIndex++;
        } while ((options[optIndex] != '\0') && ((':' == options[optIndex]) || (';' == options[optIndex])));
    }

    return optIndex;
}
