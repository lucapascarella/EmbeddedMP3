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

#ifndef OPTLIST_H
#define	OPTLIST_H

#include "Option.hpp"
#include <stdint.h>
#include <stdbool.h>
#include <list>

#define    OL_NOINDEX    -1        /* this option has no arguement */

typedef struct argument_t {
    char *argument;
    int argIndex;
    struct argument_t *nextArgument;
} argument_t;

typedef struct option_t {
    char option;
    //char *argument;
    //int argIndex;
    int argNumber;
    struct argument_t *nextArgument;
    struct option_t *next;
} option_t;

typedef struct return_t {
    struct option_t *opt;
    struct argument_t *arg;
} return_t;

class Optlist {
private:
    int argc;
    std::list<Option*> optionList;


public:
    Optlist(void);
    bool createOptionList(int argc, char * argv[], const char *options);
    argument_t *MakeArg(void);
    option_t *MakeOpt(const char option, char *const argument, const int index);
    void FreeArgList(argument_t *list);
    int MatchOption(const char argument, const char * options);
    char * getArgumentFromOption(char option);
    ~Optlist(void);

};

#endif	/* OPTLIST_H */

