#include "atm_slave_comm.h"

// own project
#include "gpio_definitions_felix.h"
#include "gpio_definitions_james.h"
#include "gpio_definitions_lucas.h"
#include "gpio_definitions_mayson.h"

#include "config.h"

#include "custom_utils.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#ifdef VALVE_SERVER_JAMES


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

bool read_bits_u16(uint16_t* data, uint8_t count_bits, TickType_t t0, TickType_t timeout_difference) {
    *data = 0;
    uint16_t the_bit = 1;
    while (count_bits != 0) {
        --count_bits;

        gpio_set_level(JAMES_ESP_TO_ATM_CLOCK, 1); // ready for receive!

        while (gpio_get_level(JAMES_ATM_TO_ESP_CLOCK) == 0) { // wait for send confirm
            if (xTaskGetTickCount() - t0 > timeout_difference) {
                ESP_LOGI(logging_tag, "timeout #4236985");
                return false;
            }
            wait_to_not_be_busy;
            //wait until clock in is HIGH
        }
        int received_bit = gpio_get_level(JAMES_ATM_TO_ESP_DATA);

        gpio_set_level(JAMES_ESP_TO_ATM_CLOCK, 0); // received successfully (confirmation)!

        while (gpio_get_level(JAMES_ATM_TO_ESP_CLOCK) == 1) { // wait for slave clock down.
            if (xTaskGetTickCount() - t0 > timeout_difference) {
                ESP_LOGI(logging_tag, "timeout #4236971");
                return false;
            }
            wait_to_not_be_busy;
            //wait until clock in is LOW
        }
        if (received_bit) *data |= the_bit;
        the_bit <<= 1;
    }
    return true;
}

#endif // VALVE_SERVER_JAMES


