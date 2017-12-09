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

void Optlist::createOptionList(int argc, char * argv[], const char *options) {

    Option *option;
    int argcIndex, argIndex, optIndex, argLen;
    char *ptr;

    for (argcIndex = 1; argcIndex < argc; argcIndex++) {
        argLen = custom_strlen(argv[argcIndex]);
        if ('-' == argv[argcIndex][0] && argLen > 1) {
            // It may a true option because starts with dash '-'
            for (argIndex = 1; argIndex < argLen; argIndex++) {
                optIndex = MatchOption(argv[argcIndex][argIndex], options);
                if (options[optIndex] == argv[argcIndex][argIndex]) {
                    // We found the matching option
                    if (':' == options[optIndex + 1]) {
                        // The option found should have a text argument
                        argIndex++;
                        if (custom_strlen(argv[argcIndex]) > argIndex) {
                            // No space between argument and option
                            option = new Option(true, options[optIndex], argv[argcIndex][argIndex - 1], &(argv[argcIndex][argIndex]), argcIndex, true);
                            optionList.push_back(option);
                        } else if (argcIndex + 1 < argc) {
                            // There must be space between the argument option
                            argcIndex++;
                            option = new Option(true, options[optIndex], argv[argcIndex - 1][argIndex - 1], argv[argcIndex], argcIndex, true);
                            optionList.push_back(option);
                        } else {
                            // Some error here
                            option = new Option(false, '\0', argv[argcIndex][argIndex - 1], NULL, argIndex - 1, true);
                            optionList.push_back(option);
                        }
                    } else if (';' == options[optIndex + 1]) {
                        // TODO
                    } else {
                        // The found option does not require a text argument
                        option = new Option(true, options[optIndex], argv[argcIndex][argIndex]);
                        optionList.push_back(option);
                    }
                } else {
                    // Option not expected
                    option = new Option(false, '\0', argv[argcIndex][argIndex], NULL, argIndex, false);
                    optionList.push_back(option);
                }
            }
        } else {
            // It may an error or a special option '&' that does not requires dash '-'
            if ((ptr = (char*) custom_memchr((void*) options, '&', custom_strlen((char*) options))) != NULL) {
                // It is a required special option '&'
                option = new Option(true, (ptr - options), '&', argv[argcIndex], argcIndex, true);
                optionList.push_back(option);
            } else {
                // It is unwanted argument, save it for debugging reason
                option = new Option(false, '\0', '!', argv[argcIndex], argcIndex, false);
                optionList.push_back(option);
            }
        }
    }
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

bool Optlist::isOptionPresent(char option) {

    std::list<Option*>::iterator it;

    for (it = optionList.begin(); it != optionList.end(); it++)
        if ((*it)->getGivenOption() == option)
            return true;
    return false;
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
