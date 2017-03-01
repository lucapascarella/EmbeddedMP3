
#include "test.h"

Test::Test(int x, int y) {
    gx = x;
    gy = y;
}

int Test::testIt() {
    return gx + gy;
}