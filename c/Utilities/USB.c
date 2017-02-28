
#include "Utilities/USB.h"

// Global variables
BOOL USBSerialEmulatorOpened = FALSE;

// Local static variables
USB_CDC_CONFIG cdc;

void USBCustomInit(void) {
    cdc.rxHead = cdc.rxTail = 0;
}

void USBCDCCustomTaskHandler() {

    BYTE USB_Out_Buffer[CDC_DATA_OUT_EP_SIZE], USB_In_Buffer[CDC_DATA_IN_EP_SIZE];
    BYTE numBytesRead, i;

    // The DTR Signal is automatically raised by PuTTY terminal
    // Instead Hercules doesn't have this automatic function, but there is a specific button
    // To enable the functionality, uncomment the following line in usb_config.h:
    // #define USB_CDC_SUPPORT_DTR_SIGNALING
    // Also in usb_function_cdc.c there is an optional code individuated by USB_CDC_SUPPORT_DTR_SIGNALING
    // in this point we manage the CDC Signal enabling a global variable to indicate the new state
    // Also in usb_function_cdc.c there is unused statment mInitDTRPin();
    if (USBSerialEmulatorOpened) {
        if (USBUSARTIsTxTrfReady()) {
            for (i = 0; i < CDC_DATA_OUT_EP_SIZE && cdc.txHead != cdc.txTail; i++) {
                // If buffer is not empty take the first byte in it
                USB_Out_Buffer[i] = cdc.txBuf[cdc.txHead];
                cdc.txHead = (cdc.txHead + 1) & (USB_CDC_BUFFER_SIZE - 1);
            }
            if (i > 0)
                putUSBUSART(USB_Out_Buffer, i);
        }

        // Read the whole USB endpoint
        numBytesRead = getsUSBUSART(USB_In_Buffer, CDC_DATA_IN_EP_SIZE);
        for (i = 0; i < numBytesRead; i++) {
            if (((cdc.rxTail + 1) & (USB_CDC_BUFFER_SIZE - 1)) == cdc.rxHead) {
                // Discard the first byte in FIFO queue
                cdc.rxHead = (cdc.rxHead + 1) & (USB_CDC_BUFFER_SIZE - 1);
            }
            cdc.rxBuf[cdc.rxTail] = USB_In_Buffer[i]; // store bytes in buffer
            cdc.rxTail = (cdc.rxTail + 1) & (USB_CDC_BUFFER_SIZE - 1);
        }
    }
}

WORD USBRead(CHAR8 *buffer, WORD count) {
    int i = 0;
    while (cdc.rxHead != cdc.rxTail && i < count) {
        // If buffer is not empty take the first byte in it
        buffer[i++] = cdc.rxBuf[cdc.rxHead];
        cdc.rxHead = (cdc.rxHead + 1) & (USB_CDC_BUFFER_SIZE - 1);
    }
    return i;
}

WORD USBWrite(char *buffer, int count) {
    int i = 0;

    for (i = 0; i < count; i++) {
        if (((cdc.txTail + 1) & (USB_CDC_BUFFER_SIZE - 1)) == cdc.txHead) {
            // Discard the first byte in FIFO queue
            cdc.txHead = (cdc.txHead + 1) & (USB_CDC_BUFFER_SIZE - 1);
        }
        cdc.txBuf[cdc.txTail] = buffer[i]; // store bytes in buffer
        cdc.txTail = (cdc.txTail + 1) & (USB_CDC_BUFFER_SIZE - 1);
    }
    return i;
}
