
#include "atm_slave_comm.h"

#include "gpio_definitions_james.h"

#ifdef VALVE_SERVER_JAMESXXX
#define wait_to_not_be_busy vTaskDelay(1) /// otherwise we block other tasks and watchdog kills the whole system -> reboot(?)


bool send_bits_u8(uint8_t data, uint8_t count_bits, TickType_t t0, TickType_t timeout_difference) {
    while (count_bits != 0) {
        --count_bits;

        gpio_set_level(JAMES_ESP_TO_ATM_DATA, data % 2);
        // data set!
        data = data / 2;
        gpio_set_level(JAMES_ESP_TO_ATM_CLOCK, 1);
        // clock set: ready for the receiver to read data bit
        while (gpio_get_level(JAMES_ATM_TO_ESP_CLOCK) == 1) {
            if (xTaskGetTickCount() - t0 > timeout_difference) {
                ESP_LOGI(logging_tag, "timeout #3611133");
                return false;
            }
            wait_to_not_be_busy;
            //wait until clock in is LOW
        }
        // receiver has read
        gpio_set_level(JAMES_ESP_TO_ATM_DATA, 0);
        gpio_set_level(JAMES_ESP_TO_ATM_CLOCK, 0);

        while (gpio_get_level(JAMES_ATM_TO_ESP_CLOCK) == 0) {
            if (xTaskGetTickCount() - t0 > timeout_difference) {
                ESP_LOGI(logging_tag, "timeout #3731134");
                return false;
            }
            wait_to_not_be_busy;
            //wait until clock in is HIGH
        }
        // receiver has confirmed end of bit.
    }
    return true;
}
#endif // VALVE_SERVER_JAMES
