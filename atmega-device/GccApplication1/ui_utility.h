/*
 * ui_utility.h
 *
 * Created: 30.05.2021 08:46:18
 *  Author: F-NET-ADMIN
 */ 


#ifndef UI_UTILITY_H_
#define UI_UTILITY_H_

#include "input.h"
#include "display.h"
#include "time_utilities.h"

inline bool require_key_pressed_for_ds(uint8_t key, uint8_t deci_seconds = 20){
	uint8_t waited = 0;
	while (waited < deci_seconds){
		++waited;
		sleep(100);
		uint16_t per8tage = uint16_t(waited) * 8 / deci_seconds;
		uint16_t led_config = ((0x0001 << 8) - 1) ^ ((0x0001 << per8tage) - 1);
		set_led(static_cast<uint8_t>(led_config));
		if (!get_button(key)) return false;
	}
	all_blink(8,100);
	return true;
}


#endif /* UI_UTILITY_H_ */