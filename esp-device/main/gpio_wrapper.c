#include "gpio_wrapper.h"

// own project
#include "gpio_definitions_felix.h"
#include "gpio_definitions_james.h"
#include "gpio_definitions_lucas.h"
#include "gpio_definitions_mayson.h"

#include "config.h"

// libraries
#include "driver/gpio.h"

void gpio_init(void) {

    //ifndef .. throw compile time error #####

    if (GPIO_OUTPUT_LANES != 0) {
        gpio_config_t output_lanes_config;
        //disable interrupt
        output_lanes_config.intr_type = GPIO_INTR_DISABLE;
        //set as output mode
        output_lanes_config.mode = GPIO_MODE_OUTPUT;
        //bit mask of the pins that you want to set,e.g.GPIO15/16
        output_lanes_config.pin_bit_mask = GPIO_OUTPUT_LANES;
        //disable pull-down mode
        output_lanes_config.pull_down_en = 0;
        //disable pull-up mode
        output_lanes_config.pull_up_en = 0;
        //configure GPIO with the given settings
        gpio_config(&output_lanes_config);
    }

    if (GPIO_INPUT_LANES != 0) {
        gpio_config_t input_lanes_config;
        //disable interrupt
        input_lanes_config.intr_type = GPIO_INTR_DISABLE;
        //set as input mode
        input_lanes_config.mode = GPIO_MODE_INPUT;
        //bit mask of the pins that you want to set,e.g.GPIO15/16
        input_lanes_config.pin_bit_mask = GPIO_INPUT_LANES;
        //disable pull-down mode
        input_lanes_config.pull_down_en = 0;
        //enable pull-up mode
        input_lanes_config.pull_up_en = GPIO_PULLUP_ENABLE;
        //configure GPIO with the given settings
        gpio_config(&input_lanes_config);
    }

}


