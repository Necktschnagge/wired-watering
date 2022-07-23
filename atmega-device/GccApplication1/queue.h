/* 
* queue.h
*
* Created: 08.08.2020 16:09:52
* Author: F-NET-ADMIN
*/


#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "hardware_entities.h"
#include "time_utilities.h"

#include <stdint.h>

class queue_item {
	public:
	uint8_t ventile;
	uint8_t minutes;
};

class queue {
	public:
	queue_item items[6];
	uint8_t valid_items { 0 };
	uint64_t switch_off_time{ 0 };
	
	bool running{ false }; // true <=> any ventile is on or will be turned on immediately
	
	public:
	
	inline bool full(){
		return valid_items == 6;
	}
	
	void add(uint8_t ventile, uint8_t minutes);
	
	void execute();
}; //queue

extern queue the_queue;

#endif //__QUEUE_H__
