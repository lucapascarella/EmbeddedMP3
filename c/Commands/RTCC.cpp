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

#include "Utilities/Optlist.hpp"

RTCC::RTCC(void) : CommandBase() {
    calculateNameLength();
}

int RTCC::command(int argc, char **argv) {

    uint16_t rtccYear;
    uint8_t rtccMon, rtccMday, rtccHour, rtccMin, rtccSec;
    int rtn;
    Optlist *opt;

    opt = new Optlist();
    return_t *optList;
    optList = opt->createOptionList(argc, argv, options);


    if (argc == 1) {
        //rtccGetDateAndTime(&rtccYear, &rtccMon, &rtccMday, &rtccHour, &rtccMin, &rtccSec);
        printf("%d/%d/%d %02d:%02d:%02d\r\n", rtccMday, rtccMon, rtccYear + 1980, rtccHour, rtccMin, rtccSec);
        rtn = 0;
    } else if (argc == 7) {
        // To do add the set functionality
        rtccMday = this->atolmm(opt->getArgumentFromOption('D'), 1, 31, 1);
        rtccMon = this->atolmm(argv[2], 1, 12, 1);
        rtccYear = this->atolmm(argv[3], 2010, 2200, 2017) - 1980;

        rtccHour = this->atolmm(argv[4], 0, 23, 1);
        rtccMin = this->atolmm(argv[5], 0, 59, 1);
        rtccSec = this->atolmm(argv[6], 0, 59, 1);

        //RtccSetDateAndTime();
        rtn = 0;
    } else {
        this->argumnetsProblem();
        rtn = -1;
    }
    return rtn;
}

const char * RTCC::getCommandOptions(void) {
    return options;
}

const char * RTCC::getCommandName(void)  {
    return name;
}
