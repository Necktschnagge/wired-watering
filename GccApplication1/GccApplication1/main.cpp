
#include "timer0.h"
#include "display.h"
#include "input.h"
#include "time_utilities.h"
#include "queue.h"
#include "pump.h"
#include "ui_utility.h"

#include <avr/eeprom.h>
/******
CURRENT PIN USAGE:::

PC: 5:0 -> blue_status_led : relay : Ventiles[3]
PD: input buttons
PB: + led mixed: 7seg + single led
******/

void pin_init(){
	DDRC |=  0b00011111; // safe_on led : relay: ventile2, ventile1, ventile0
	//DDRC &= ~0b00000100;
	PORTC = 0;

	DDRD &= ~0b11111111; // in button
	PORTD = 0b11111111; // pull-up internal on.

	DDRB = 0b11111111; // single leds.
	PORTB = 0;
}

human_clock& the_clock{ human_clock::instance() };


class timer {
	static constexpr uint8_t START_TIME_WINDOW_MINUTES{ 15 };
	static constexpr uint16_t MINUTES_OF_A_DAY{ 12*60 };

	public:
	
	static bool global_timer_enable;
	uint8_t ventile{ 255 }; // 255 for invalid.
	uint8_t minutes{ 1 };
	uint16_t minute_at_day{ 0 };
	uint64_t last_executed{ 0 };
	
	inline bool invalid(){ return ventile == 255; }
	
	void execute(){
		if (!global_timer_enable) return;
		if (invalid()) return;
		bool enable = ((MINUTES_OF_A_DAY + the_clock.minute_of_day() - minute_at_day) % MINUTES_OF_A_DAY < START_TIME_WINDOW_MINUTES) && (the_clock.now() > last_executed);
		if (!enable) return;
		
		the_queue.add(ventile, minutes);
		last_executed = the_clock.now() + human_clock::MINUTE * (START_TIME_WINDOW_MINUTES + 1);
	}
	
	
	uint8_t get_two_bit_ventile_code() const {
		return ventile<3 ? ventile : 3;
	}
	
	uint32_t compressed_eeprom_data() const {
		// 8 bit duration				23:16
		// 2 bit ventile				15:14
		// 3 bit unused					13:11
		// 11 bit minute at day			10:00
		return
		(static_cast<uint32_t>(minutes) << 16) +
		(static_cast<uint32_t>(get_two_bit_ventile_code()) << 14) +
		static_cast<uint32_t>(minute_at_day & 0x7FF);
	}
	
	void load_from_eeprom_data(uint32_t bit_code){
		minutes = (bit_code & 0x00FF0000) >> 16;
		ventile = (bit_code & 0x0000C000) >> 14;
		if (ventile > 2) ventile = 255;
		minute_at_day = (bit_code & 0x000007FF);
		if (minute_at_day >= 12*60) minute_at_day = 0;
		
		last_executed = the_clock.now() + human_clock::MINUTE * (START_TIME_WINDOW_MINUTES + 1);
	}
	
};

bool timer::global_timer_enable{ true };

constexpr uint8_t NUM_TIMERS { 30 };

using timer_array = timer[NUM_TIMERS];

timer_array all_timers;

// little endian saving
constexpr uint8_t* get_eeprom_address_for_timer(uint8_t timer_index){
	return reinterpret_cast<uint8_t*>(uint16_t(0) + uint16_t(3) * timer_index);
	static_assert(sizeof(void*)==2,"error");
}

uint32_t get_bit_code_for_timer_at(const uint8_t* eeprom_address){
	uint8_t byte0 = eeprom_read_byte(eeprom_address);
	uint8_t byte1 = eeprom_read_byte(eeprom_address+1);
	uint8_t byte2 = eeprom_read_byte(eeprom_address+2);
	return uint32_t(byte0) + (uint32_t(byte1) << 8) + (uint32_t(byte2) << 16);
}

void load_all_timers_from_eeprom(){
	for(uint8_t i = 0; i < NUM_TIMERS; ++i){
		all_timers[i].load_from_eeprom_data(get_bit_code_for_timer_at(get_eeprom_address_for_timer(i)));
	}
}

void eeprom_write_byte_if_changed(uint8_t* eeprom_address, uint8_t value){
	const uint8_t old = eeprom_read_byte(eeprom_address);
	if (value == old) return;
	eeprom_write_byte(eeprom_address, value);
}

void save_timer_to_eeprom(const timer& t, uint8_t* eeprom_address){
	const uint32_t bit_code = t.compressed_eeprom_data();
	const uint8_t byte0 = (bit_code & 0x0000FF);
	const uint8_t byte1 = (bit_code & 0x00FF00) >> 8;
	const uint8_t byte2 = (bit_code & 0xFF0000) >> 16;
	eeprom_write_byte_if_changed(eeprom_address, byte0);
	eeprom_write_byte_if_changed(eeprom_address + 1, byte1);
	eeprom_write_byte_if_changed(eeprom_address + 2, byte2);
}

bool timer_to_eeprom_has_changes(const timer& t, uint8_t* eeprom_address){
	const uint32_t bit_code = t.compressed_eeprom_data();
	const uint8_t byte0 = (bit_code & 0x0000FF);
	const uint8_t byte1 = (bit_code & 0x00FF00) >> 8;
	const uint8_t byte2 = (bit_code & 0xFF0000) >> 16;
	return !(eeprom_read_byte(eeprom_address) == byte0 &&
	eeprom_read_byte(eeprom_address + 1) == byte1 &&
	eeprom_read_byte(eeprom_address + 2) == byte2);
}


void save_all_timers_to_eeprom(){
	for(uint8_t i = 0; i < NUM_TIMERS; ++i){
		save_timer_to_eeprom(all_timers[i],get_eeprom_address_for_timer(i));
	}
}

bool has_any_timer_changes_to_eeprom(){
	for(uint8_t i = 0; i < NUM_TIMERS; ++i){
		if (timer_to_eeprom_has_changes(all_timers[i],get_eeprom_address_for_timer(i))) return true;
	}
	return false;
}


void init_timers(){
	for (uint8_t i = 0; i < NUM_TIMERS; ++i){
		all_timers[i].ventile = 255;
		all_timers[i].minute_at_day = 0;
		all_timers[i].minutes = 0;
	}
}

void execute_timers(){
	for (uint8_t i = 0; i < NUM_TIMERS; ++i)
	{
		all_timers[i].execute();
	}
}



void manual_queue_entry(){
	constexpr uint8_t abort_buttons{ 0b11101000 };
	
	auto abort_blink = []{
		for (uint8_t i = 0; i < 16; ++i){
			set_led(0xFF * (i%2));
			sleep(250);
		}
		
	};
	
	uint8_t ticks{ 8 };
	while (get_button(3)){
		set_led(0xFF & ( (1ul << ticks) - 1));
		if (ticks == 0) {
			abort_blink();
			the_queue.switch_off_time = the_clock.now();
			return;
		}
		sleep(400);
		--ticks;
	}
	
	sleep(300);
	//{ if (get_buttons() & abort_buttons) return; }
	if (the_queue.full()){
		abort_blink();
		return; // abort
	}
	
	// choose ventile
	for (uint8_t i = 0; i < 10; ++i){
		set_led(0b11100000 * (i%2));
		sleep(300);
		{ if (get_buttons() & abort_buttons) return; }
	}
	uint8_t ventile{ 0 };
	if (!get_number(ventile, 1, abort_buttons, 30)) return;
	
	if (ventile > 2){
		abort_blink();
		return; // abort
	}

	// choose time
	for (uint8_t i = 0; i < 8; ++i){
		set_led(1 << ((3*i) % 8));
		sleep(300);
		{ if (get_buttons() & abort_buttons) return; }
	}
	uint8_t time{ 15 };
	if (!get_number(time, 1, abort_buttons, 30)) return;
	
	the_queue.add(ventile, time);
	set_led(0xFF);
	sleep(2000);
}

void config_change(){
	constexpr uint8_t exit_buttons { 0b10000100 };
	const auto exit_delta = human_clock::SECOND * 25;
	const uint16_t push_down_time_ms{ 3000 };

	auto reset_auto_exit = [&](human_clock::time_type& auto_exit){
		auto_exit = the_clock.now() + exit_delta;
	};
	
	auto right_to_left_blink = [reset_auto_exit](human_clock::time_type& auto_exit){
		for (uint8_t i = 1; i < 9; ++i){
			set_led((uint16_t(1) << i) - 1);
			sleep(250);
		}
		reset_auto_exit(auto_exit);
	};
	
	auto check_long_push_down = [reset_auto_exit, right_to_left_blink](uint8_t button, bool& flag_to_change, human_clock::time_type& auto_exit){
		uint16_t already_waited_ms = 0;
		while(already_waited_ms < push_down_time_ms){
			if (!get_button(button)) return;
			sleep(30);
			already_waited_ms += 30;
			reset_auto_exit(auto_exit);
		}
		flag_to_change = !flag_to_change;
		right_to_left_blink(auto_exit);
	};

	
	bool& global_timer_enable { timer::global_timer_enable };
	bool& the_2_hr_reactivate{ pump::recovery_enable };
	human_clock::time_type auto_exit{ the_clock.now() + exit_delta };
	
	right_to_left_blink(auto_exit);
	while (true){
		// refresh led
		uint8_t status{
			static_cast<uint8_t>(
			(0b00000111 * global_timer_enable) |
			(0b00111000 * the_2_hr_reactivate)
			)
		};
		const auto now{ the_clock.now()};
		status = (status * ( now % 2)) |
		(0b10000000 * !(now % 2));
		set_led(status);
		
		// capture input
		check_long_push_down(0,global_timer_enable,auto_exit);
		check_long_push_down(1,the_2_hr_reactivate,auto_exit);
		
		// check exit
		if ((get_buttons() & exit_buttons) || auto_exit < the_clock.now() ){
			all_blink(8,250);
			return;
		}
	}
}

void save_timer_to_eeprom_ui(){
	const bool ch = has_any_timer_changes_to_eeprom();
	set_led(0x80 | (7 << (3 * ch)));
	save_all_timers_to_eeprom();
	sleep(2000);
	left_to_right_blink();
}

void load_all_timers_from_eeprom_ui(){
	set_led(0b10111);
	load_all_timers_from_eeprom();
	sleep(2000);
	left_to_right_blink();
}

void edit_specific_timer(uint8_t index_timer_selected){
	const uint8_t edit_ventile{ 0b11100000 };
	const uint8_t edit_hours{ 0b00011000};
	const uint8_t edit_quartile{ 0b00000110};
	const uint8_t edit_minutes{ 0b11 };
	const uint8_t edit_duration_minutes{ 0b10100101 };
	
	const uint8_t exit_buttons{ 0b10000000 };
	
	const auto exit_delta = human_clock::SECOND * 25;
	human_clock::time_type auto_exit{ the_clock.now() + exit_delta };
	
	auto reset_auto_exit = [&auto_exit](){
		auto_exit = the_clock.now() + exit_delta;
	};
	
	uint8_t edit_select{ 0 };
	
	const auto update_led = [&](){
		switch (edit_select) {
			case 0: set_led(edit_ventile); break;
			case 1: set_led(edit_hours); break;
			case 2: set_led(edit_quartile); break;
			case 3: set_led(edit_minutes); break;
			case 4: set_led(edit_duration_minutes); break;
		}
	};
	
	while (true){
		update_led();
		
		//check button
		if(get_button(0)){ // UP
			edit_select = (edit_select + 1) % 5;
			update_led(); sleep(500);
			reset_auto_exit();
			continue;
		}
		if(get_button(1)){ // DOWN
			edit_select = (edit_select + 4) % 5;
			update_led(); sleep(500);
			reset_auto_exit();
			continue;
		}
		if(get_button(2)){ // OK
			set_led(0xFF); sleep(800);
			if(edit_select == 0){
				uint8_t v{ all_timers[index_timer_selected].ventile };
				get_number(v,1,0,0);
				all_timers[index_timer_selected].ventile = v < 3 ? v : 0xFF;
			}
			if(edit_select == 1){
				uint8_t h = all_timers[index_timer_selected].minute_at_day / 60;
				get_number(h,1,0,0);
				if (h>23) h=0;
				all_timers[index_timer_selected].minute_at_day = (all_timers[index_timer_selected].minute_at_day % 60) + uint16_t(60) * h;
			}
			if(edit_select == 2){
				uint8_t q = (all_timers[index_timer_selected].minute_at_day % 60) / 15;
				get_number(q,1,0,0);
				if (q>3) q=0;
				all_timers[index_timer_selected].minute_at_day =
				(all_timers[index_timer_selected].minute_at_day % 15) +
				(all_timers[index_timer_selected].minute_at_day / 60) * 60 +
				uint16_t(15) * q;
			}
			if(edit_select == 3){
				uint8_t m = all_timers[index_timer_selected].minute_at_day % 15;
				get_number(m,1,0,0);
				if (m>14) m=0;
				all_timers[index_timer_selected].minute_at_day =
				(all_timers[index_timer_selected].minute_at_day / 15) * 15 + m;
			}
			if (edit_select ==4){
				uint8_t d = all_timers[index_timer_selected].minutes;
				get_number(d,1,0,0);
				all_timers[index_timer_selected].minutes = d;
			}
			left_to_right_blink();
			reset_auto_exit();
			continue;
		}
		
		// check exit
		if ((get_buttons() & exit_buttons) || auto_exit < the_clock.now() ){
			all_blink(8,200);
			return;
		}
	}
}

void edit_timers(){
	const uint8_t entry_button = 0;
	const uint8_t exit_buttons = 0b10000000;
	const auto exit_delta = human_clock::SECOND * 25;

	for(uint8_t i = 0; i < 9; ++i){
		if (!get_button(entry_button)){
			all_blink(10, 100);
			return;
		}
		sleep(250);
		set_led((uint16_t(1) << i)-1);
	}
	set_led(0xFF);
	while(get_buttons()){}
	left_to_right_blink();
	
	human_clock::time_type auto_exit{ the_clock.now() + exit_delta };

	auto reset_auto_exit = [&]{
		auto_exit = the_clock.now() + exit_delta;
	};

	uint8_t index_timer_selected{ 0 };
	// UP DOWN
	
	// OK -> edit
	// ventile
	// time
	
	const auto update_led = [&](){
		timer& current_timer = all_timers[index_timer_selected];
		uint8_t ventile_code = current_timer.invalid() ? 0xE0 :(1 << (7-current_timer.ventile));
		set_led(index_timer_selected + 1 + ventile_code);
	};
	
	while(true){
		//update led
		update_led();
		
		//check button
		if(get_button(0)){ // UP
			index_timer_selected = (index_timer_selected + 1) % NUM_TIMERS;
			update_led(); sleep(500);
			reset_auto_exit();
			continue;
		}
		if(get_button(1)){ // DOWN
			index_timer_selected = (index_timer_selected + NUM_TIMERS - 1) % NUM_TIMERS;
			update_led(); sleep(500);
			reset_auto_exit();
			continue;
		}
		if(get_button(2)){ // OK
			set_led(0xFF); sleep(800);
			edit_specific_timer(index_timer_selected);
			reset_auto_exit();
			continue;
		}
		
		if (get_button(3)){ // SAVE to eeprom
			if (!require_key_pressed_for_ds(3)) continue;;
			save_timer_to_eeprom_ui();
			reset_auto_exit();
			continue;
		}
		
		if (get_button(4)){ // LOAD from eeprom
			if (!require_key_pressed_for_ds(4)) continue;;
			load_all_timers_from_eeprom_ui();
			reset_auto_exit();
			continue;
		}		
		// check exit
		if ((get_buttons() & exit_buttons) || auto_exit < the_clock.now() ){
			all_blink(8,200);
			return;
		}
		
	}
	
	// entered successfully.
	
	
}

void check_manual_terminal(){
	static uint8_t test_ventile{ 0 };
	
	if ((get_buttons() & 0b00100001) == 0b00100001){ // S key + UP key -> edit timers
		return edit_timers();
	}
	
	if (get_button(7)){ // off
		the_pump.manual(false);
		return;
	}
	if (get_button(6)){ // on
		the_pump.manual(true);
		return;
	}
	if (get_button(5)){ // unsafe-on
		the_pump.manual_unsafe_on();
		return;
	}
	
	if (get_button(4)){ // ventile test
		if (the_queue.running) return;
		
		++test_ventile;
		test_ventile %= 6;
		turn_off_ventiles();
		if (test_ventile % 2){
			turn_on_ventile(test_ventile / 2);
			the_queue.switch_off_time = the_clock.now() + 1 * human_clock::MINUTE;
		}
		sleep(700);
		return;
	}
	
	if (get_button(3)){ // F key for custom queue entry.
		return manual_queue_entry();
	}
	
	if (get_button(2)){ // prog config menu
		return config_change();
	}
}

void update_led(){

	uint64_t now = the_clock.now();

	bool pump_status_safe = (the_pump.safe_on && the_pump.manual_on) || (the_pump.booting() && (now % 0b10)) || (the_pump.safe_on && (!the_pump.manual_on) && (now % 0b100));
	set_safe_on_led(pump_status_safe);

	now %= human_clock::DAY;
	uint8_t now_hour = now / human_clock::HOUR;
	uint8_t part_of_hour = (now % human_clock::HOUR) / (human_clock::HOUR / 8);
	if (part_of_hour > 7) part_of_hour = 7;
	if (now_hour > 23) now_hour = 24;

	set_led((now_hour << 3) | part_of_hour);
}


void apply_preconfigured_timers(){
	
	// cucumber
	all_timers[0].ventile = 0;
	all_timers[0].minutes = 15;
	all_timers[0].minute_at_day = 7 * 60 + 0; // 7:00

	//all_timers[1].ventile = 0;
	//all_timers[1].minutes = 12;
	//all_timers[1].minute_at_day = 12 * 60 + 0;
	
	all_timers[2].ventile = 0;
	all_timers[2].minutes = 10;
	all_timers[2].minute_at_day = 18 * 60 + 0; // 18:00

	// peas
	all_timers[3].ventile = 2;
	all_timers[3].minutes = 10;
	all_timers[3].minute_at_day = 7 * 60 + 1; // 7:01
	
	//all_timers[4].ventile = 2;
	//all_timers[4].minutes = 5;
	//all_timers[4].minute_at_day = 12 * 60;
	
	//all_timers[5].ventile = 2;
	//all_timers[5].minutes = 5;
	//all_timers[5].minute_at_day = 15 * 60;
	
	all_timers[6].ventile = 2;
	all_timers[6].minutes = 5;
	all_timers[6].minute_at_day = 18 * 60 + 1; // 18:01
	

	// night drain / blue berries:

	/*
	all_timers[7].ventile = the_pump.drain_ventile;
	all_timers[7].minute_at_day = 24 * 60;
	all_timers[7].minutes = 10;
	
	all_timers[8].ventile = the_pump.drain_ventile;
	all_timers[8].minute_at_day = 21 * 60;
	all_timers[8].minutes = 1;
	
	all_timers[9].ventile = the_pump.drain_ventile;
	all_timers[9].minute_at_day = 3 * 60 + 30;
	all_timers[9].minutes = 1;
	*/

}

void super_init_timers(){
	init_timers();
	
	left_to_right_blink();
	const bool skip_load_eeprom = get_buttons();
	
	if (skip_load_eeprom){
		set_led(0b11000000);
		apply_preconfigured_timers();
		} else {
		set_led(0b00000111);
		load_all_timers_from_eeprom();
	}
	set_safe_on_led(true);
	sleep(3000);
	set_safe_on_led(false);
	set_led(0xFF);
	sleep(3000);
	set_led(0);
}


int main(void)
{
	
	pin_init();
	super_init_timers();
	the_clock.set();
	
	
	while (true)
	{
		the_pump.execute();
		the_queue.execute();
		execute_timers();
		check_manual_terminal();
		update_led();
	}
	
	
	
}