#include "FatFS/ff.h"
#include "Commands.h"
#include "CommandLineInterpreter.h"
#include "Utilities/printer.h"

typedef enum {
    SM_CAT_HOME = 0,
    SM_CAT_OPEN_FILE,
    SM_CAT_READ_FILE,
    SM_CAT_WRITE,
    SM_CAT_CLOSE_FILE,
    SM_CAT_ERROR,
    SM_CAT_END,
} CAT_SM;

#define CAT_BUFFER_SIZE        1024

typedef struct __PACKED {
    CAT_SM sm;
    FRESULT fres;
    FIL *fp;
    FILINFO *finfo;
    UINT read;
    BOOL printBinFile;
    UINT altBuf;
    BYTE buf[2][CAT_BUFFER_SIZE];
} CAT;

int Cat(int argc, char **argv) {

    extern FIL ftmp1;
    extern FILINFO finfo;
    static CAT cat;
    int rtn = 0;
    int i = 0;
    char *p;

    if (argc < 2) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 2 || argc == 3) {
        rtn = 1;
        switch (cat.sm) {
            case SM_CAT_HOME:
                if (argc == 3 && strcmp(argv[2], "-bin") == 0)
                    cat.printBinFile = TRUE;
                else
                    cat.printBinFile = FALSE;

                p = argv[1];
                if (cat.printBinFile || (p = strrchr(argv[1], '.')) != NULL) {
                    // Copy the extensions in local buf
                    while ((cat.buf[0][i++] = toupper(*p++)) != '\0' && i < CAT_BUFFER_SIZE);
                    cat.buf[0][CAT_BUFFER_SIZE - 1] = '\0';
                    if (cat.printBinFile || strstr(cat.buf[0], "TXT") != NULL || strstr(cat.buf[0], "INI") != NULL || strstr(cat.buf[0], "PLS") != NULL) {
                        // Test file existence
                        cat.finfo = &finfo;
                        cat.fp = &ftmp1;
                        cat.fres = f_stat(argv[1], cat.finfo);
                        if (cat.fres == FR_NO_FILE) {
                            printf("File does not exist\r\n");
                        }
                        cat.sm = (cat.fres == FR_OK) ? SM_CAT_OPEN_FILE : SM_CAT_ERROR;
                    } else {
                        CliOnlyTextual();
                        cat.sm = SM_CAT_END;
                    }
                } else {
                    CliOnlyTextual();
                    cat.sm = SM_CAT_END;
                }
                break;

            case SM_CAT_OPEN_FILE:
                // Open the current directory
                cat.fres = f_open(cat.fp, argv[1], FA_READ);
                cat.sm = (cat.fres == FR_OK) ? SM_CAT_READ_FILE : SM_CAT_ERROR;
                printf("Size: %u byte\r\n", cat.finfo->fsize);
                cat.altBuf = 0;
                break;

            case SM_CAT_READ_FILE:
                cat.fres = f_read(cat.fp, cat.buf[cat.altBuf % 2], CAT_BUFFER_SIZE, &cat.read);
                if (cat.fres != FR_OK)
                    cat.sm = SM_CAT_ERROR;
                else if (cat.read == 0)
                    cat.sm = SM_CAT_CLOSE_FILE;
                else
                    cat.sm = SM_CAT_WRITE;
                break;

            case SM_CAT_WRITE:
                if (UartWriteDirectly(cat.buf[cat.altBuf % 2], cat.read) == 0)
                    break;
                cat.altBuf++;
                //ConsolWrite(cat.buf, cat.read);
                cat.sm = SM_CAT_READ_FILE;
                break;

            case SM_CAT_CLOSE_FILE:
                cat.fres = f_close(cat.fp);
                printf("\r\n");
                cat.sm = (cat.fres == FR_OK) ? SM_CAT_END : SM_CAT_ERROR;
                break;

            case SM_CAT_ERROR:
                put_rc(cat.fres);
                cat.sm = SM_CAT_END;
                break;

            case SM_CAT_END:
                cat.sm = SM_CAT_HOME;
                rtn = 0;
                break;
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return rtn;
}
