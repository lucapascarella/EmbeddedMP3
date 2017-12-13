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

#include "Commands/RTCC.hpp"
#include "Utilities/RTCC.h"
#include "Utilities/ArgsParser.hpp"

RTCC::RTCC(CLI *cli) : CommandBase() {
    this->cli = cli;
    calculateNameLength();
}

const char * RTCC::getCommandOptions(void) {
    return options;
}

const char * RTCC::getCommandName(void) {
    return name;
}

int RTCC::command(void) {

    const char requiredOptions[] = "hmsDMY";
    uint16_t rtccYear;
    uint8_t rtccMon, rtccMday, rtccHour, rtccMin, rtccSec;
    int rtn;

    switch (numOfOpt) {
        case 0:
            rtccGetDateAndTime(&rtccYear, &rtccMon, &rtccMday, &rtccHour, &rtccMin, &rtccSec);
            printf("\r\n%d/%d/%d %02d:%02d:%02d", rtccMday, rtccMon, rtccYear, rtccHour, rtccMin, rtccSec);
            rtn = COMMAND_BASE_TERMINATED;
            break;

        case 6:
            if (this->checkRequiredOptions(requiredOptions)) {
                rtccHour = this->atolmm(opt->getFirstArgumentForOption('h'), 0, 23, 1);
                rtccMin = this->atolmm(opt->getFirstArgumentForOption('m'), 0, 59, 1);
                rtccSec = this->atolmm(opt->getFirstArgumentForOption('s'), 0, 59, 1);
                rtccMday = this->atolmm(opt->getFirstArgumentForOption('D'), 1, 31, 1);
                rtccMon = this->atolmm(opt->getFirstArgumentForOption('M'), 1, 12, 1);
                rtccYear = this->atolmm(opt->getFirstArgumentForOption('Y'), 2010, 2200, 2017);
                rtccSetDateAndTime(rtccYear, rtccMon, rtccMday, rtccHour, rtccMin, rtccSec);
                rtn = COMMAND_BASE_TERMINATED;
            } else {
                this->printUnexpectedOptions(requiredOptions);
                rtn = COMMAND_BASE_ERROR;
            }
            break;

        default:
            this->printUnexpectedNumberOfOptions();
            rtn = COMMAND_BASE_ERROR;
            break;
    }

    return rtn;
}

int RTCC::verbosePrintf(int level, bool reprint, const char * fmt, ...) {
    int rtn;
    va_list ap;
    va_start(ap, fmt);
    rtn = cli->verbosePrintfWrapper(level, reprint, fmt, ap);
    va_end(ap);

    return rtn;
}

int RTCC::helper(void){
    return COMMAND_BASE_TERMINATED;
}
