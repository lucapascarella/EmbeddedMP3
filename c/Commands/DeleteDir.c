
#include "FatFS/ff.h"
#include "Commands.h"
#include "Compiler.h"
#include "CommandLineInterpreter.h"
#include "Utilities/printer.h"

typedef enum {
    SM_RMDIR_HOME = 0,
    SM_RMDIR_OPEN_DIR,
    SM_RMDIR_READ_DIR,
    SM_RMDIR_DELETE_DIR,
    SM_RMDIR_DELETE_FILE,

    SM_RMDIR_END,
    SM_RMDIR_ERROR

} RMDIR_SM;

typedef struct __PACKED {
    UINT i;
    DIR dir;
    FILINFO fno;
    //UINT alignError;
    void *prev;
    void *next;
} DEL_DIR;

int DeleteDir(int argc, char **argv2) {

    extern TCHAR Lfname[_MAX_LFN + 1];
    int rtn = 0;
    static char* path;
    static DEL_DIR *dd;
    static RMDIR_SM sm = 0;
    static FRESULT fr;
    static UINT j;

    if (argc < 2) {
        CliTooFewArgumnets(argv2[0]);
    } else if (argc == 2) {
        // Check the existence of the resource
        if ((fr = f_unlink(argv2[1])) != FR_OK) {

            if (fr == FR_NO_FILE)
                // File not found
                CliNoSuchFileOrDirectory();
            else if (fr == FR_DENIED)
                printf("The directory is not empty\r\n");
            else
                // Print the error message
                put_rc(fr);
        } else {
            CliCreateFileListOfFilesEntry();
        }
    } else if (argc == 3) {
        rtn = 1;
        switch (sm) {
            case SM_RMDIR_HOME:
                dd = NULL;
                if (strcmp(argv2[2], "-not-empty") == 0) {
                    dd = malloc(sizeof (DEL_DIR));
                    dd->prev = NULL;
                    dd->next = NULL;
                    //dd->fno.lfname = Lfname;
                    //dd->fno.lfsize = sizeof (Lfname);
                    path = argv2[1];
                    sm++;
                } else {
                    printf("Wrong arguments\r\n");
                    sm = SM_RMDIR_ERROR;
                }
                break;

            case SM_RMDIR_OPEN_DIR:
                //path = argv[1];
                if ((fr = f_opendir(&dd->dir, path)) == FR_OK) {
                    for (dd->i = 0; path[dd->i]; dd->i++);
                    path[dd->i++] = '/';
                    sm++;
                } else {
                    sm = SM_RMDIR_ERROR;
                }
                break;

            case SM_RMDIR_READ_DIR:
                if ((fr = f_readdir(&dd->dir, &dd->fno)) == FR_OK) {
                    if (!dd->fno.fname[0]) {
                        // Current directory is empty
                        sm = SM_RMDIR_DELETE_DIR;
                    } else if (dd->fno.fname[0] == '.') {
                        // Skip dot entry
                        sm = SM_RMDIR_READ_DIR;
                    } else {
                        // Copy the file name or directory name in the working buffer
                        j = 0;
                        do {
                            path[dd->i + j] = dd->fno.fname[j];
                        } while (dd->fno.fname[j++]);

                        // Check file or dircetory entry
                        if (dd->fno.fattrib & AM_DIR) {
                            dd->next = (DEL_DIR*) malloc(sizeof (DEL_DIR));
                            ((DEL_DIR*) dd->next)->prev = dd;
                            dd = dd->next;
                            //dd->fno.lfname = Lfname;
                            //dd->fno.lfsize = sizeof (Lfname);
                            // Dovrebbe essere ricorsivo
                            sm = SM_RMDIR_OPEN_DIR;
                        } else
                            sm = SM_RMDIR_DELETE_FILE;
                    }
                } else {
                    sm = SM_RMDIR_ERROR;
                }
                break;

            case SM_RMDIR_DELETE_DIR:
                path[--dd->i] = '\0';
                f_closedir(&dd->dir);
                if ((fr = f_unlink(path)) == FR_OK) {
                    if (dd->prev != NULL) {
                        dd = dd->prev;
                        free(dd->next);
                        sm = SM_RMDIR_READ_DIR;
                    } else {
                        sm = SM_RMDIR_END;
                    }
                } else
                    sm = SM_RMDIR_ERROR;
                break;

            case SM_RMDIR_DELETE_FILE:
                if ((fr = f_unlink(path)) == FR_OK)
                    sm = SM_RMDIR_READ_DIR;
                else
                    sm = SM_RMDIR_END;
                break;

            case SM_RMDIR_ERROR:
                if (fr == FR_NO_FILE || fr == FR_NO_PATH) {
                    // File not found
                    CliNoSuchFileOrDirectory();
                    sm = SM_RMDIR_END;
                    break;
                }
                // Print the error message
                put_rc(fr);

            case SM_RMDIR_END:
                //path[--i] = '\0';
                if (dd != NULL)
                    f_closedir(&dd->dir);
                if (dd->prev != NULL) {
                    dd = dd->prev;
                    free(dd->next);
                    sm = SM_RMDIR_END;
                } else {
                    if (dd != NULL)
                        free(dd);
                    rtn = 0;
                    sm = SM_RMDIR_HOME;
                }
                break;
        }
    } else {
        CliTooManyArgumnets(argv2[0]);
    }
    return rtn;
}