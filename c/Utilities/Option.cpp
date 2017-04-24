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

#include "Utilities/Option.hpp"
#include "Utilities/Optlist.hpp"

Option::Option(void) {
    this->option = '\0';
    this->argument = NULL;
    this->argIndex = 0;
}

Option::Option(char option) {
    this->option = option;
    this->argument = NULL;
    this->argIndex = 0;
}

Option::Option(char option, char *argument, int index) {
    this->option = option;
    this->argument = argument;
    this->argIndex = index;
}

char Option::getOption(void) {
    return this->option;
}

char * Option::getArgument(void) {
    return this->argument;
}

int Option::getArgumentIndex(void) {
    return this->argIndex;
}

Option::~Option(void) {

}
