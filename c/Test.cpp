
#include "Utilities/printer.h"
#include "Test.hpp"

Test::Test(int x, int y) {
    gx = x;
    gy = y;
}

int Test::testIt(void) {
    return gx + gy;
}

void Test::callException(int b) {

    verbosePrintf(VER_DBG, "Value of: %d", b);

    try {
        this->testThrow(b);
        // Because the function throws an exception,
        // the rest of the code in this block will not
        // be executed
    } catch (char const* pch) {
        // will react on exceptions
        // of type char const*
        char string[10];
        memcpy(string, pch, strlen(pch));
    }
}

void Test::testThrow(int a) {
    this->gx = a;
    throw "Throw exception";
}