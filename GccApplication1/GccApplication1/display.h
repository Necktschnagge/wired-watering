/*
* display.h
*
* Created: 02.08.2020 13:14:51
*  Author: F-NET-ADMIN
*/


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "time_utilities.h"

#include <avr/io.h>


inline void set_led(uint8_t x){
	PORTB = x;
}

inline void set_safe_on_led(bool on){
	PORTC = (PORTC & ~0b10000) | (on * 0b10000);
}

static constexpr uint32_t PUBLISH_TIME{ 3000 };

inline void publish(uint8_t led_code){ set_led(led_code); sleep(PUBLISH_TIME); }

inline void left_to_right_blink(uint32_t step_sleep = 200){
	uint8_t last{ 0 };
	for (uint8_t i = 0; i < 8; ++i){
		set_led(last);
		sleep(step_sleep);
		last |= uint8_t(1) << (7-i);
	}
	set_led(last);
	sleep(step_sleep);
}

inline void all_blink(uint8_t count, uint16_t step_sleep){
	uint16_t left = uint16_t(count * 2);
	while(left){
		--left;
		set_led(0xFF * (left % 2));
		sleep(step_sleep);
	}
}

#endif /* DISPLAY_H_ */