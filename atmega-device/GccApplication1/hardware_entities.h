/* 
* hardware_entities.h
*
* Created: 08.08.2020 16:12:57
* Author: F-NET-ADMIN
*/


#ifndef __HARDWARE_ENTITIES_H__
#define __HARDWARE_ENTITIES_H__

#include <avr/io.h>
#include <stdint.h>

inline bool any_ventile_on(){
	return PINC & 0b111;
}

inline void turn_on_ventile(uint8_t id){
	if (any_ventile_on()) return; // for overload safety.
	if (id > 2) return;
	PORTC |= (0b1 << id);
}

inline void turn_off_ventiles(){
	PORTC &= ~0b111;
}

inline void turn_on_pump_relay(){
	PORTC |= 0b1000;
}

inline void turn_off_pump_relay(){
	PORTC &= ~0b1000;
}

inline bool is_pump_relay_on_raw(){
	return (PINC & 0b1000);
}


#endif //__HARDWARE_ENTITIES_H__
