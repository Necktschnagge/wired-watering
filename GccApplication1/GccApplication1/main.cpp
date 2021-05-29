
#include "timer0.h"
#include "display.h"
#include "input.h"
#include "time_utilities.h"
#include "queue.h"
#include "pump.h"

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
		bool enable = (the_clock.minute_of_day() == minute_at_day) && (the_clock.now() > last_executed);
		if (!enable) return;
		
		the_queue.add(ventile, minutes);
		last_executed = the_clock.now() + human_clock::MINUTE * 8;
	}
	
};

bool timer::global_timer_enable{ true };

constexpr uint8_t NUM_TIMERS { 20 };

using timer_array = timer[NUM_TIMERS];

timer_array all_timers;

void init_timers(){
	for (uint8_t i = 0; i < NUM_TIMERS; ++i){
		all_timers[i].ventile = 255;
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

	auto reset_auto_exit = [](human_clock::time_type& auto_exit){
		auto_exit = the_clock.now() + exit_delta;
	};
	
	auto all_on_off_8_times = []{
		for (uint8_t i = 0; i < 16; ++i){
			set_led(0xFF * (i%2));
			sleep(250);
		}
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
			all_on_off_8_times();
			return;
		}
	}
}

void check_manual_terminal(){
	static uint8_t test_ventile{ 0 };
	
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
	
	if (get_button(3)){
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

int main(void)
{
	
	pin_init();
	init_timers();
	the_clock.set();
	
	/*
	pre-configured timers:
	*/
	
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
	
	while (true)
	{
		the_pump.execute();
		the_queue.execute();
		execute_timers();
		check_manual_terminal();
		update_led();
	}
	
	
	
}