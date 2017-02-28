
#include "FatFS/ff.h"
#include "Commands.h"
#include "CommandLineInterpreter.h"
#include "Utilities/printer.h"
//#include "Delay/Delay.h"
#include "Utilities/RTCC.h"

typedef enum {
    SM_LIST_HOME = 0,
    SM_LIST_OPEN_DIR,
    SM_LIST_READ_DIR,
    SM_LIST_CLOSE_DIR,
    SM_LIST_NO_PATH,
    SM_LIST_ERROR,
    SM_LIST_END,
} LIST_SM;

#define LIST_BUFFER_SIZE        512

typedef struct __PACKED {
    LIST_SM sm;
    FRESULT fres;
    BYTE buf[LIST_BUFFER_SIZE];
    DIR *dir;
    FILINFO *finfo;
    WORD countTotObj;
    WORD countFile;
    WORD countDir;
    DWORD totalSize;

    struct {
        BYTE list : 1;
        BYTE hidden : 1;
        BYTE relative : 1;
        BYTE start : 1;
    } flags;
} LIST;

int List(int argc, char **argv) {

    extern FILINFO finfo;
    extern DIR dir;
    static LIST list;
    int rtn;
    FAT_TIME fat_time;
    long size, integer, decimal;
    double sized;
    char units;

    if (list.flags.start == FALSE) {
        rtn = 0;
        list.flags.list = FALSE;
        list.flags.hidden = FALSE;
        list.flags.relative = FALSE;
        if (argc == 1) {
            // Do nothing
        } else if (argc == 2) {
            if (strchr(argv[1], '-') != NULL) {
                if (strchr(argv[1], 'l') != NULL)
                    list.flags.list = TRUE;
                if (strchr(argv[1], 'a') != NULL)
                    list.flags.hidden = TRUE;
            } else {
                list.flags.relative = TRUE;
            }
        } else if (argc == 3) {
            list.flags.relative = TRUE;
            if (strchr(argv[1], '-') != NULL) {
                if (strchr(argv[1], 'l') != NULL)
                    list.flags.list = TRUE;
                if (strchr(argv[1], 'a') != NULL)
                    list.flags.hidden = TRUE;
            } else {
                printf("Wrong arguments\r\n");
                return rtn;
            }
        } else {
            CliTooManyArgumnets(argv[0]);
            return rtn;
        }
    }

    rtn = 1;
    switch (list.sm) {
        case SM_LIST_HOME:
            list.flags.start = TRUE;
            list.fres = FR_OK;
            size = strlen(argv[argc - 1]);
            // Delete the very last '/' to avoid the FatFS FR_INVALID_NAME error
            if (size != 1 && argv[argc - 1][size - 1] == '/')
                argv[argc - 1][size - 1] = '\0';
            if (list.flags.relative)
                strncpy(list.buf, argv[argc - 1], LIST_BUFFER_SIZE);
            else
                list.fres = f_getcwd(list.buf, LIST_BUFFER_SIZE);
            list.dir = &dir;
            list.finfo = &finfo;
            list.sm = (list.fres == FR_OK) ? SM_LIST_OPEN_DIR : SM_LIST_ERROR;
            break;

        case SM_LIST_OPEN_DIR:
            // Open the given directory name
            list.fres = f_opendir(list.dir, list.buf);
            // Reset file counter
            list.countFile = list. countDir = list.countTotObj = list.totalSize = 0;
            list.sm = (list.fres == FR_OK) ? SM_LIST_READ_DIR : (list.fres == FR_NO_PATH) ? SM_LIST_NO_PATH : SM_LIST_ERROR;
            break;

        case SM_LIST_READ_DIR:
            // Read contents
            list.fres = f_readdir(list.dir, list.finfo);
            if (list.fres == FR_OK) {
                // Break on end of dir
                if (list.finfo->fname[0] == 0) {
                    // Current directory is empty
                    list.sm = SM_LIST_CLOSE_DIR;
                } else if (list.flags.list) {
                    if (list.flags.hidden == TRUE || !(finfo.fattrib & AM_HID)) {
                        size = list.finfo->fsize;
                        if (size < 1024) {
                            sized = size;
                            units = ' ';
                        } else if (size < 1048576) {
                            sized = (double) (size / 1024.0);
                            units = 'k';
                        } else if (size < 1073741824) {
                            sized = size / 1048576.0;
                            units = 'M';
                        } else {
                            sized = size / 1073741824.0;
                            units = 'G';
                        }
                        integer = sized;
                        decimal = (sized - integer) * 100;

                        fat_time.word.data = finfo.fdate;
                        fat_time.word.time = finfo.ftime;

                        printf("%s %4d.%-2d %cByte  %d/%d/%d\t%02d:%02d:%02d\t%s\r\n", ByteToFatAttributes(finfo.fattrib), integer, decimal, units, (fat_time.fields.year + 1980), fat_time.fields.month, fat_time.fields.day, fat_time.fields.hour, fat_time.fields.min, (fat_time.fields.sec * 2), GetFileNamePointer(&finfo));

                        list.countTotObj++;
                        if (finfo.fattrib & AM_DIR) {
                            list.countDir++;
                        } else {
                            list.countFile++;
                            list.totalSize += finfo.fsize;
                        }
                    }
                } else {
                    // Skip dot entry
                    if ((!(list.finfo->fattrib & AM_HID) || list.flags.hidden) && list.finfo->fname[0] != '.') {
                        // It is a file or directory
                        printf("%s\r\n", GetFileNamePointer(list.finfo));
                    }
                }
            } else {
                list.sm = SM_LIST_ERROR;
            }
            break;

        case SM_LIST_CLOSE_DIR:
            // Close the current directory
            list.fres = f_closedir(list.dir);

            if (list.flags.list) {
                if (list.countTotObj == 0)
                    printf("The directory is empty\r\n");
                else
                    printf("Total itme(s): %d\r\n", list.countTotObj);

                printf("%4u File(s)%8lu kBytes\r\n%4u Dir(s)\r\n", list.countFile, list.totalSize / 1024, list.countDir);
                //                if (f_getfree("0", (DWORD*) & totalSize, &fs) == FR_OK)
                //                    printf(", %10lu Mbytes free\r\n", totalSize * fs->csize / 2 / 1024);
            }
            list.sm = (list.fres == FR_OK) ? SM_LIST_END : SM_LIST_ERROR;
            break;

        case SM_LIST_NO_PATH:
            printf("Not found\r\n");
            list.sm = SM_LIST_END;
            break;

        case SM_LIST_ERROR:
            put_rc(list.fres);
            list.sm = SM_LIST_END;
            break;

        case SM_LIST_END:
            list.sm = SM_LIST_HOME;
            list.flags.start = FALSE;
            rtn = 0;
            break;
    }
    //    } else {
    //        CliTooManyArgumnets(argv[0]);
    //    }

    return rtn;
}
