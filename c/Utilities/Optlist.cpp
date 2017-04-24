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
    //optionList = NULL;
}

bool Optlist::createOptionList(int argc, char * argv[], const char *options) {

    Option *option;
    int nextArg, optIndex, argIndex;

    // Start with first argument and nothing found
    nextArg = 1;
    // Loop through all of the command line arguments
    while (nextArg < argc) {
        argIndex = 1;
        while ((custom_strlen(argv[nextArg]) > argIndex) && ('-' == argv[nextArg][0])) {
            optIndex = MatchOption(argv[nextArg][argIndex], options);
            if (options[optIndex] == argv[nextArg][argIndex]) {
                // We found the matching option
                if (':' == options[optIndex + 1]) {
                    // The option found should have a text argument
                    argIndex++;
                    if (custom_strlen(argv[nextArg]) > argIndex) {
                        // No space between argument and option
                        option = new Option(options[optIndex], argv[nextArg][argIndex - 1], &(argv[nextArg][argIndex]), nextArg, true);
                        optionList.push_back(option);
                    } else if (nextArg + 1 < argc) {
                        // There must be space between the argument option
                        nextArg++;
                        option = new Option(options[optIndex], argv[nextArg - 1][argIndex - 1], argv[nextArg], nextArg, true);
                        optionList.push_back(option);
                    } else {
                        // Some error here
                        option = new Option('\0', argv[nextArg][argIndex - 1], NULL, argIndex - 1, true);
                        optionList.push_back(option);
                    }
                    break; /* done with argv[nextArg] */
                } else {
                    // the option found does not have a text argument
                    option = new Option(options[optIndex], argv[nextArg][argIndex]);
                    optionList.push_back(option);
                }
            } else {
                // Option not expected
                option = new Option('\0', argv[nextArg][argIndex], NULL, argIndex, false);
                optionList.push_back(option);
            }
            argIndex++;
        }
        nextArg++;
    }
    return true;
}

int Optlist::MatchOption(const char argument, const char * options) {
    int optIndex = 0;

    // Attempt to find a matching option
    while ((options[optIndex] != '\0') && (options[optIndex] != argument)) {
        do {
            optIndex++;
        } while ((options[optIndex] != '\0') && ((':' == options[optIndex]) || (';' == options[optIndex])));
    }

    return optIndex;
}

char * Optlist::getFirstArgumentForOption(char option) {

    std::list<Option*>::iterator it;

    for (it = optionList.begin(); it != optionList.end(); it++)
        if ((*it)->getGivenOption() == option)
            return (*it)->getArgument();
    return NULL;
}

int Optlist::getNumberOfArgumentsForOption(char option) {

    int count;
    std::list<Option*>::iterator it;

    for (count = 0, it = optionList.begin(); it != optionList.end(); it++)
        if ((*it)->getGivenOption() == option)
            count++;
    return count;
}

Option *Optlist::getOptionNumber(uint16_t number) {

    uint16_t i;
    std::list<Option*>::iterator it;

    if (number < optionList.size()) {
        it = optionList.begin();
        i = 0;
        do {
            if (i++ == number)
                return (*it);
            it++;
        } while (it != optionList.end());
    }
    return NULL;
}

int Optlist::getNumberOfOptions(void) {
    return optionList.size();
}

Optlist::~Optlist(void) {
    std::list<Option*>::iterator it;

    // Release memory space for each option
    for (it = optionList.begin(); it != optionList.end(); it++)
        (*it)->~Option();
    optionList.clear();
    optionList.~list();
}
