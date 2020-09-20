#include <mbed.h>
#include <unity.h>
#include "si7210.h"

// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

void test_led_builtin_pin_number(void)
{
    // The device address
    uint8_t devAddr7Bit = 0x31U;

    // Create the I2C bus
    PinName sda = PA_10;
    PinName scl = PA_9;
    I2C i2c(sda, scl);

    // Create an si7210 object
    si7210 hall(&i2c, devAddr7Bit);
    TEST_ASSERT_EQUAL(0x1, hall.getChipId());
}

int main()
{
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    wait(2);

    UNITY_BEGIN();

    RUN_TEST(test_led_builtin_pin_number);

    UNITY_END();
}

// uint8_t i = 0;
// uint8_t max_blinks = 5;

// void loop() {
//     if (i < max_blinks)
//     {
//         RUN_TEST(test_led_state_high);
//         delay(500);
//         RUN_TEST(test_led_state_low);
//         delay(500);
//         i++;
//     }
//     else if (i == max_blinks) {
//       UNITY_END(); // stop unit testing
//     }
// }