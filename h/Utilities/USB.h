/* 
 * File:   USB.h
 * Author: Luca
 *
 * Created on 28 agosto 2015, 15.21
 */

#ifndef USB_H
#define	USB_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "Compiler.h"
#include "HardwareProfile.h"
#include "USB/usb_function_cdc.h"
    
#define USB_CDC_BUFFER_SIZE     512
    
typedef struct __attribute__((__packed__)) {
    CHAR rxBuf[USB_CDC_BUFFER_SIZE];
    WORD rxHead;
    WORD rxTail;
    CHAR txBuf[USB_CDC_BUFFER_SIZE];
    WORD txHead;
    WORD txTail;
} USB_CDC_CONFIG;
    
void USBCDCCustomTaskHandler();
WORD USBRead(CHAR8 *buffer, WORD count);
WORD USBWrite(char *buffer, int count);


#ifdef	__cplusplus
}
#endif

#endif	/* USB_H */

