/* 
* queue.cpp
*
* Created: 08.08.2020 16:09:52
* Author: F-NET-ADMIN
*/


#include "queue.h"

#include "pump.h"


queue the_queue;

void queue::add(uint8_t ventile, uint8_t minutes)
{
	if (full()) return; // buffer full;

	if (!running){
		++the_pump.requiries;
	}
	items[valid_items].minutes = minutes;
	items[valid_items].ventile = ventile;
	++valid_items;
	running = true;
}

void queue::execute()
{
	if (!the_pump.safe_on) return;
	if (any_ventile_on()){
		if (human_clock::instance().now() > switch_off_time) {
			turn_off_ventiles();
			if (!valid_items) {
				if (running) --the_pump.requiries;
				running = false;
			}
		}
		return;
	}
	if (!valid_items) return;
	turn_on_ventile(items[0].ventile);
	switch_off_time = human_clock::instance().now() + items[0].minutes * human_clock::MINUTE;
	for (uint8_t i = 0; i + 1 < 6; ++i) items[i] = items[i+1];
	--valid_items;
}
