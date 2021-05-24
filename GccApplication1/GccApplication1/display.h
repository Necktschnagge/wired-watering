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


#endif /* DISPLAY_H_ */