/*
 * time_utilities.cpp
 *
 * Created: 02.08.2020 13:29:18
 *  Author: F-NET-ADMIN
 */ 

#include "time_utilities.h"

#include "display.h"
#include "input.h"
#include "timer0.h"

void human_clock::set(){
	human_clock_seconds = 0;
	uint8_t hour{ 12 };
	uint8_t minute { 30 };
	
	for (uint8_t i = 0; i < 9; ++i){
		uint8_t code = 1 << i;
		code = code + (code == 0) * 0xFF;
		set_led(code);
		sleep(400);
	}
	sleep(2000);
	
	publish(0b11111000);
	get_number(hour, 1);
	
	publish(0b111);
	get_number(minute, 5);
	
	hour %= 24;
	minute %= 60;
	
	set_led(0b11111111);
	sleep(2000);
	set_led(0);
	
	human_clock_seconds = hour * human_clock::HOUR + minute * human_clock::MINUTE;
	
	reset_timer_0();
	activate_timer0_compare_match_interrupt_A(32, human_clock::inc_human_clock, true);
	start_timer_prescaler_1024();
}

