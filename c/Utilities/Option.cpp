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
    this->givenOption = '\0';
    this->foundOption = '\0';
    this->argument = NULL;
    this->argIndex = 0;
    this->requiredOption = false;
}

Option::Option(char reqOpt, char opt) {
    this->givenOption = reqOpt;
    this->foundOption = opt;
    this->argument = NULL;
    this->argIndex = 0;
    this->requiredOption = false;
}

Option::Option(char given, char found, char *argument, int argIndex, bool required) {
    this->givenOption = given;
    this->foundOption = found;
    this->argument = argument;
    this->argIndex = argIndex;
    this->requiredOption = required;
}

char Option::getGivenOption(void) {
    return this->givenOption;
}

char Option::getFoundOption(void) {
    return this->foundOption;
}

bool Option::getRequiredOption(void) {
    return this->requiredOption;
}

char * Option::getArgument(void) {
    return this->argument;
}

int Option::getArgumentIndex(void) {
    return this->argIndex;
}

Option::~Option(void) {

}
