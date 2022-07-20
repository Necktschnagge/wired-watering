/*
 * input.h
 *
 * Created: 02.08.2020 13:23:47
 *  Author: F-NET-ADMIN
 */ 


#ifndef INPUT_H_
#define INPUT_H_

#include <avr/io.h>


inline bool get_button(uint8_t i){
	return !(PIND & (1 << i));
}

inline uint8_t get_buttons(){
	return ~PIND;
}

bool get_number(uint8_t& x, uint8_t step = 1, uint8_t abort_on = 0, uint8_t abort_sec = 0);

#endif /* INPUT_H_ */