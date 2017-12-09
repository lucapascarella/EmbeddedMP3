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
    this->requiredOption = false;
    this->givenOption = '\0';
    this->foundOption = '\0';
    this->argument = NULL;
    this->argIndex = 0;
    this->requiredArgument = false;
}

Option::Option(bool req, char given, char found) {
    this->requiredOption = req;
    this->givenOption = given;
    this->foundOption = found;
    this->argument = NULL;
    this->argIndex = 0;
    this->requiredArgument = false;
}

Option::Option(bool reqOpt, char given, char found, char *argument, int argIndex, bool requiredArg) {
    this->requiredOption = reqOpt;
    this->givenOption = given;
    this->foundOption = found;
    this->argument = argument;
    this->argIndex = argIndex;
    this->requiredArgument = requiredArg;
}

bool Option::isOptionExpected(void) {
    return this->requiredOption;
}

char Option::getGivenOption(void) {
    return this->givenOption;
}

char Option::getFoundOption(void) {
    return this->foundOption;
}

bool Option::isArgumentRequired(void) {
    return this->requiredArgument;
}

char * Option::getArgument(void) {
    return this->argument;
}

int Option::getArgumentIndex(void) {
    return this->argIndex;
}

Option::~Option(void) {

}
