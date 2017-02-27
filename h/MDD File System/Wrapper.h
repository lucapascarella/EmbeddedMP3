/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        Wrapper.h
 * Dependencies:    Compiler.h GenericTypeDefs.h HardwareProfile.h
 * Processor:       PIC32MX250F128B
 * Compiler:        Microchip XC32 v1.11a or higher
 * Company:         LP Systems
 * Author:	    Luca Pascarella luca.pascarella@gmail.com
 * Web Site:        www.lucapascarella.it
 *
 * Software License Agreement
 *
 * Copyright (C) 2012-2013 LP Systems  All rights reserved.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 * File Description: Encoder and Decoder state finite machine
 * Change History: In progress
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release (1 September 2013, 16.00)
 *
 ********************************************************************/
#ifndef __WRAPPER_H
#define __WRAPPER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "MDD File System/FSIO.h"



#ifdef FILE
#undef FILE
#endif
#define FILE                FSFILE

#ifdef fclose
#undef fclose
#endif
#define fclose(a)           _fclose(a)

#ifdef fopen
#undef fopen
#endif
#define fopen(a,b)          FSfopen(a,b)

#ifdef fprintf
#undef frpintf
#endif
#define fprintf(...)        FSfprintf(__VA_ARGS__)

#ifdef fgets
#undef  fgets
#endif
#define fgets(a,b,c)        _fgets(a,b,c)
char *_fgets(char *buf, int n, FILE *fp);

#ifdef fread
#undef  fread
#endif
#define fread(a,b,c,d)      FSfread(a,b,c,d)

#ifdef fflush
#undef  fflush
#endif
#define fflsuh(a)           FSfflush(a)

#ifdef fwrite
#undef  fwrite
#endif
#define fwrite(a,b,c,d)     FSfwrite(a,b,c,d)

#ifdef fseek
#undef  fseek
#endif
#define fseek(a,b,c)        FSfseek(a,b,c)

#ifdef ftell
#undef  ftell
#endif
#define ftell(a)            FSftell(a)

#ifdef unlink
#undef  unlink
#endif
#define unlink(a)           _unlink(a)
BOOL _unlink(char *name);

#ifdef fssek
#undef  fssek
#endif
#define fseek(a,b,c)        FSfseek(a,b,c)



char *getPointer(SearchRec *rec);
BOOL frename(char *oldname, char *newname);


#endif