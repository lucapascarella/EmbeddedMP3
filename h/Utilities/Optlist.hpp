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

/*
 * Option rules
 * a    -> no arguments
 * a:   -> one argument
 * a;   -> multiple arguments
 * &    -> free argument
 * !    -> accidental arguments
 * 
 * Option examples
 * a:
 * cmd -a12
 * cmd -a 12
 * 
 * ab:
 * cmd -a -b 12
 * cmd -a -b12
 * cmd -ab12
 * 
 * &
 * cmd file.txt
 * 
 * a&
 * cmd -a file.txt
 * cmd file.txt -a
 */

#define    OL_NOINDEX    -1        /* this option has no arguement */

class Optlist {
private:
    std::list<Option*> optionList;

private:
    int MatchOption(const char argument, const char * options);
    
public:
    Optlist(void);
    void createOptionList(int argc, char * argv[], const char *options);
    char * getFirstArgumentForOption(char option);
    bool isOptionPresent(char option);
    int getNumberOfArgumentsForOption(char option);
    Option * getOptionNumber(uint16_t number);
    int getNumberOfOptions(void);
    ~Optlist(void);

};

#endif	/* OPTLIST_H */

