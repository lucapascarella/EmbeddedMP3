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

#ifndef OPTION_H
#define	OPTION_H

#include <stdint.h>
#include <stdbool.h>

class Option {
private:
    bool requiredOption;
    char givenOption;
    char foundOption;
    char *argument;
    int argIndex;
    bool requiredArgument;

public:
    Option(void);
    Option(bool req, char given, char found);
    Option(bool reqOpt, char given, char found, char *argument, int argIndex, bool requiredArg);
    bool isOptionExpected(void);
    char getGivenOption(void);
    char getFoundOption(void);
    bool isArgumentRequired(void);
    char * getArgument(void);
    int getArgumentIndex(void);
    ~Option(void);
};

#endif	/* OPTION_H */

