/*
 * input.cpp
 *
 * Created: 02.08.2020 13:59:32
 *  Author: F-NET-ADMIN
 */ 

#include "input.h"

#include "time_utilities.h"
#include "display.h"

bool get_number(uint8_t& x, uint8_t step, uint8_t abort_on, uint8_t abort_sec){
	/*
	button 2 -> ok
	button 1 -> down
	button 0 -> up
	*/
	uint64_t stamp{ 0 };
	while (true)
	{
		set_led(x);
		sleep(700);
		stamp  = human_clock::instance().now();
		bool ok, down, up;
		while (true){
			if (abort_sec) if (human_clock::instance().now() - stamp >= abort_sec) return false;
			if (get_buttons() & abort_on) return false;
			ok = get_button(2);
			up = get_button(0);
			down = get_button(1);
			if ( ok || up || down ) break;
		}
		if (ok){
			return true;
		}
		if (up){
			if (255 - x < step) {
				x = 255;
				} else {
				x += step;
			}
			continue;
		}
		if (down){
			if (step > x) {
				x = 0;
				} else {
				x -= step;
			}
			continue;
		}
	}
	
}
