/* 
 * File:   CLI.cpp
 * Author: luca
 * 
 * Created on April 11, 2017, 12:53 PM
 */

#include "CLI.hpp"
#include "Utilities/printer.h"
#include "Utilities/Utilities.h"
#include "Commands/CommandBase.h"
#include <iterator>

CLI::CLI(void) {

    //    int i;

    cmd = NULL;

    //    // Reset all string buffer
    //    for (i = 0; i < CLI_MAX_BUF_SIZE; i++)
    //        cl.cmd[i] = cl.tmp[i] = '\0';
    //
    //    // Reset other command line variables
    //    cl.cmdlen = cl.cmdi = cl.tmpi = cl.tmplen = 0;
    //
    //    ////    // Reset the commands iterator 
    //    ////    for (i = 0; i < NUMBER_OF_COMMANDS; i++)
    //    ////        cmd_iterator[i].cmd_len = strlen(commands[i].name) - 1;
    //
    //    // Create two hidden files with the list of commands and entry list of current directory
    //    this->CliCreateFileListOfCommands();
    //    this->CliCreateFileListOfFilesEntry();

}

void CLI::registerCommand(CommandBase &cb) {
    commandList.push_back(cb);
}

CommandBase * CLI::searchCommand(char *name) {

    int len;
    std::list<CommandBase>::iterator it;

    len = strlen(name);
    for (it = commandList.begin(); it != commandList.end(); it++)
        if (len == it->getCommandNameLength() && strcmp(name, it->getCommandName()) == 0)
            return &(*it);
    return NULL;
}

void CLI::cliTaskHadler(void) {
    int rnt;

    switch (sm) {
        case CLI_SM_HOME:
            break;
    }

    if (cmd != NULL) {
        rnt = cmd->taskCommand();
    }
}

//bool CLI::CliCreateFileListOfCommands(void) {
//
//    FIL *fp;
//    FRESULT fres;
//    int i;
//    bool rtn;
//
//    // Allocate enough space for FIL structure
//    custom_malloc((void**) fp, sizeof (FIL));
//
//    rtn = true;
//    // Create a temporary file where will create a list of commands
//    if ((fres = f_open(fp, temporaryFileCommands, FA_WRITE | FA_CREATE_ALWAYS)) == FR_OK) {
//        // Changes the properties of the temporary file to hide it
//        if ((fres = f_chmod(temporaryFileCommands, AM_HID, AM_HID)) == FR_OK) {
//            // Prints all commands in the file
//            for (i = 0; i < numberOfCommands; i++) {
//                f_printf(fp, "%s\n", commands[i].name);
//            }
//        } else {
//            verbosePrintf("Error: %s", string_rc(fres));
//            rtn = false;
//        }
//        // Close the temporary file
//        if ((fres = f_close(fp)) != FR_OK) {
//            // Unable to close the temporary file
//            verbosePrintf("Error: %s", string_rc(fres));
//            rtn = false;
//        }
//    } else {
//        // Unable to create temporary file.
//        verbosePrintf("Error: %s", string_rc(fres));
//        rtn = false;
//    }
//
//    custom_free((void**) &fp);
//
//    return rtn;
//}
//
//bool CLI::CliCreateFileListOfFilesEntry(void) {
//
//    FILINFO *finfo;
//    FIL *fp;
//    DIR *dir;
//    FRESULT fres;
//
//    // Allocate enough space for FILINFO, FIL, and DIR structures
//    custom_malloc((void**) finfo, sizeof (FILINFO));
//    custom_malloc((void**) fp, sizeof (FIL));
//    custom_malloc((void**) dir, sizeof (DIR));
//
//    // Create a temporary file where will create a list of files
//    if ((fres = f_open(fp, temporaryFileEntryList, FA_WRITE | FA_CREATE_ALWAYS)) == FR_OK) {
//        // Changes the properties of the temporary file to hide it
//        put_rc(f_chmod(temporaryFileEntryList, AM_HID, AM_HID));
//
//        // Flush the hidden property
//        put_rc(f_sync(fp));
//
//        // Get the current directory name
//        put_rc(f_getcwd(cl.tmp, CLI_MAX_DIR_SIZE));
//
//        // Open the directory
//        if (put_rc(f_opendir(&dir, cl.tmp))) {
//
//            while (TRUE) {
//                // Read a directory item
//                if (!put_rc(f_readdir(&dir, &finfo)))
//                    break;
//                // Break on end of dir
//                if (finfo.fname[0] == 0)
//                    break;
//                // Ignore dot entry
//                if (finfo.fname[0] == '.')
//                    continue;
//
//                if (!(finfo.fattrib & AM_HID)) {
//                    // It is a file
//                    f_printf(fp, "%s\n", GetFileNamePointer(&finfo));
//                }
//            }
//            put_rc(f_closedir(&dir));
//        }
//
//        // Try to close temporary file
//        if (!put_rc(f_close(fp))) {
//            // Unable to close the temporary file
//            return FALSE;
//        }
//        return TRUE;
//
//    } else {
//        // Unable to create temporary file.
//        verbosePrintf("Error: %s", string_rc(fres));
//        return FALSE;
//    }
//}

CLI::~CLI(void) {
}

