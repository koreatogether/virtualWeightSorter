#include "unity/unity.h"
#include "FakeArduino.h"
#include "../Utils.h"
#include "TestGlobals.h" // Include TestGlobals.h

void test_addrToString_converts_address_correctly(void) {
    DeviceAddress testAddress = {0x28, 0x58, 0x82, 0x84, 0x00, 0x00, 0x00, 0x0E};
    char expectedString[] = "2858:8284:0000:000E";
    char actualString[24];
    addrToString(testAddress, actualString);
    TEST_ASSERT_EQUAL_STRING(expectedString, actualString);
}

int run_utils_tests() {
    UNITY_BEGIN();
    RUN_TEST(test_addrToString_converts_address_correctly);
    return UNITY_END();
}
