
#include "Utilities/printer.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "Utilities/Config.h"
#include "Utilities/Uart.h"
#include "USB/usb_function_cdc.h"
#include "Utilities/Uart.h"
#include "Utilities/GPIO.h"


CIRCULAR_BUFFER txUSB;

int __putc(char c) {

    // print a single character
    SerialWrite(&c, 1);
}

int __puts(const char *p) {

    // print a string
    while (*p != 0)
        __putc(*p++);
}

int __printf(const char * fmt, ...) {

    va_list ap;
    int n = 0;
    char buf[1024];

    // If the user disable both UART and USB serial console do anything
    if (isUARTEnabled() || isUSBEnabled()) {
        va_start(ap, fmt);
        n = vsnprintf(buf, sizeof (buf), fmt, ap);
        va_end(ap);

        // Print on UART or USB port
        SerialWrite(buf, n);
    }

    return n;
}

char returnLineVerbose[] = "\r\n>";

int verbosePrintf(int level, const char * fmt, ...) {

    va_list ap;
    int n;
    char buf[1024];

    if (level <= config.console.verbose) {
        // If the user disable both UART and USB serial console do anything
        if (isUARTEnabled() || isUSBEnabled()) {
            va_start(ap, fmt);
            n = vsnprintf(buf, sizeof (buf), fmt, ap);
            va_end(ap);

            // Print on UART or USB port
            SerialWrite(buf, n);
            SerialWrite(returnLineVerbose, sizeof (returnLineVerbose));

            return n;
        }
    }

    return 0;
}

WORD SerialWrite(CHAR8 *buffer, WORD count) {

    if (config.console.port == 0) {
        // Print on UART port
        //SerialWrite(returnLine, sizeof (returnLine));
        UartWrite(buffer, count);
    } else {
        // Print on USB port
        USBAddCharToBuffer(buffer, count);
    }
}

WORD SerialRead(CHAR8 *buffer, WORD count) {

    WORD read;

    if (config.console.port == 0) {
        read = UartRead(buffer, count);
    } else {
        if ((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1))
            return 0;
        read = getsUSBUSART(buffer, count);
    }
    return read;
}

void Config(int argc, char **argv) {

    if (argc < 2) {
        printf("Serial-port: %s\r\n", config.console.port ? "USB" : "UART");
    } else if (argc == 2) {
        // Check first the existence of the passed file
        config.console.port = atoimm(argv[1], 0, 1, 0);
    } else {
        CliTooManyArgumnets(argv[0]);
    }
}

void Verbose(int argc, char **argv) {

    if (argc < 2) {
        printf("Verbose: %d\r\n", config.console.verbose);
    } else if (argc == 2) {
        // Check first the existence of the passed file
        config.console.verbose = atoimm(argv[1], 0, 3, 2);
    } else {
        CliTooManyArgumnets(argv[0]);
    }
}

void USBAddCharToBuffer(char *p, int count) {

    int i;

    for (i = 0; i < count; i++) {
        // Wait so that the buffer has at least one empty position
        //            while (((tx.put + 1) % SER_BUF_SIZE) == tx.get) {
        //                Nop();
        //            }
        txUSB.buff[txUSB.put] = p[i];
        txUSB.put = ((txUSB.put + 1) % SER_BUF_SIZE);
    }
}

BOOL USBSerialEmulatorOpened = FALSE;

void USBPrintTaskHandler() {

    // The DTR Signal is automatically raised by PuTTY terminal
    // Instead Hercules doesn't have this automatic function, but there is a specific button
    // To enable the functionality, uncomment the following line in usb_config.h:
    // #define USB_CDC_SUPPORT_DTR_SIGNALING
    // Also in usb_function_cdc.c there is an optional code individuated by USB_CDC_SUPPORT_DTR_SIGNALING
    // in this point we manage the CDC Signal enabling a global variable to indicate the new state
    // Also in usb_function_cdc.c there is unused statment mInitDTRPin();
    if (USBSerialEmulatorOpened)
        // Update the buffer only if a terminal, like PuTTY is detected.
        while (USBUSARTIsTxTrfReady() && txUSB.put != txUSB.get) {
            putUSBUSART(&txUSB.buff[txUSB.get], 1);
            txUSB.get = (txUSB.get + 1) % SER_BUF_SIZE;
        }

}