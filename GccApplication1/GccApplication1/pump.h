/* 
* pump.h
*
* Created: 08.08.2020 16:15:54
* Author: F-NET-ADMIN
*/


#ifndef __PUMP_H__
#define __PUMP_H__

#include "hardware_entities.h"
#include "time_utilities.h"
#include "display.h"
#include "queue.h"

class pump {
	public:
	static constexpr uint8_t drain_ventile{ 1 };
	static bool recovery_enable;
	
	uint8_t requiries{ 0 };
	
	bool manual_on{ false };
	
	bool safe_on{ false };
	
	bool recovery_started{ false };
	
	bool booting(){ return on_boot_pending_switches != 0; };
	
	uint8_t on_boot_seconds_at_next_switch;
	
	uint8_t on_boot_pending_switches{ 0 };
	
	uint64_t time_out_manual_on{ 0 };
	
	void manual(bool on){
		if (on){
			if (!manual_on) ++requiries;
			manual_on = true;
			time_out_manual_on = human_clock::instance().now() + 8 * human_clock::HOUR;
			return;
		}
		// turn off:
		if (manual_on) --requiries;
		manual_on = false;
	}
	
	void manual_unsafe_on(){
		manual(true);
		turn_on_pump_relay();
		if (booting()){
			turn_off_ventiles();
			on_boot_pending_switches = 0;
		}
		safe_on = true;
	}
	
	void execute(){
		// recovery drain.
		if (!booting() && (!the_queue.running) && recovery_enable){
			if ((human_clock::instance().now() % (human_clock::HOUR * 2)) < human_clock::MINUTE){
				if (!recovery_started){
					recovery_started = true;
					safe_on = false;
					turn_off_pump_relay();
					++requiries;
					for (uint8_t i = 0; i < 8; ++i)
					{
						set_led(1 << (7-i));
					}
				}
			}
		}
		if ((human_clock::instance().now() % (human_clock::HOUR * 2)) > 6 * human_clock::MINUTE){
			if (recovery_started){
				recovery_started = false;
				--requiries;
			}
		}

		if (manual_on){
			if (human_clock::instance().now() > time_out_manual_on){
				manual(false);
			}
		}
		if (booting()){
			if (!requiries){ // abort booting
				turn_off_pump_relay();
				turn_off_ventiles();
				on_boot_pending_switches = 0;
				return;
			}
			turn_on_ventile(drain_ventile);
			if (( human_clock::instance().now() + 80 - on_boot_seconds_at_next_switch ) % 80 < 20){ // can switch.
				--on_boot_pending_switches;
				if ((!is_pump_relay_on_raw()) || (on_boot_pending_switches == 0)){
					turn_on_pump_relay();
					} else {
					turn_off_pump_relay();
				}
				on_boot_seconds_at_next_switch = (human_clock::instance().now() + 80 - 22) % 80; // forward switch time
				if (!on_boot_pending_switches) {
					safe_on = true;
					turn_off_ventiles();
				}
			}
			return;
		}
		// if requiries, turn on, otherwise turn off.
		if (requiries && (!is_pump_relay_on_raw())){
			turn_on_ventile(drain_ventile);
			on_boot_pending_switches = 2;
			on_boot_seconds_at_next_switch = (human_clock::instance().now() + 80 - 22) % 80;
			return;
		}
		if (!requiries){
			turn_off_pump_relay();
			safe_on = false;
			return;
		}
	}
};

extern pump the_pump;


#endif //__PUMP_H__
