
#include "timer0.h"
#include "display.h"
#include "input.h"
#include "time_utilities.h"
#include "ui_utility.h"

#include <avr/eeprom.h>
/******
CURRENT PIN USAGE:::

PC: 5:0 -> blue_status_led : relay : Ventiles[3]
PD: input buttons
PB: + led mixed: 7seg + single led
******/

void pin_init(){
	//DDRC |=  0b00011111; // safe_on led : relay: ventile2, ventile1, ventile0
	//DDRC &= ~0b00000100;
	//PORTC = 0;

	DDRC &= ~0b0011'1000; // buttons
	PORTC |= 0b0011'1000; // buttons pull-up


	// arc out
	PORTD = 0b0000'0000; // out arc 0.
	PORTB &= ~0b0000'0001;
	DDRD = 0b11111111; // out arc
	DDRB |= 0b00000001; //
}

human_clock& the_clock{ human_clock::instance() };


class timer {
	static constexpr uint8_t START_TIME_WINDOW_MINUTES{ 15 };
	static constexpr uint16_t MINUTES_OF_A_DAY{ 24*60 };

	public:
	
	static bool global_timer_enable;
	uint8_t ventile{ 255 }; // 255 for invalid.
	uint8_t minutes{ 1 };
	uint16_t minute_at_day{ 0 };
	uint64_t last_executed{ 0 };
	
	inline bool invalid(){ return ventile == 255; }
	
	bool execute(){
		if (!global_timer_enable) return false;
		if (invalid()) return false;
		bool enable = ((MINUTES_OF_A_DAY + the_clock.minute_of_day() - minute_at_day) % MINUTES_OF_A_DAY < START_TIME_WINDOW_MINUTES) && (the_clock.now() > last_executed);
		if (!enable) return false;
		//the_queue.add(ventile, minutes);
		last_executed = the_clock.now() + human_clock::MINUTE * (START_TIME_WINDOW_MINUTES + 1);
		return true;
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
		if (minute_at_day >= 24*60) minute_at_day = 0;
		
		last_executed = the_clock.now() + human_clock::MINUTE * (START_TIME_WINDOW_MINUTES + 1);
		///### last executed is set before the clock is configured. So it does not take any effect on startup.
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
		bool happened = all_timers[i].execute();
		if (happened){
			set_led(i);
			for(uint8_t i = 0; i < 41; ++i){
				set_safe_on_led(i%2);
				sleep(125);
			}
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


void apply_preconfigured_timers(){
	
	// cucumber
	all_timers[0].ventile = 0;
	all_timers[0].minutes = 15;
	all_timers[0].minute_at_day = 6 * 60 + 50; // 6:50

	all_timers[1].ventile = 0;
	all_timers[1].minutes = 15;
	all_timers[1].minute_at_day = 10 * 60 + 0; // 10:00
	
	all_timers[2].ventile = 0;
	all_timers[2].minutes = 10;
	all_timers[2].minute_at_day = 18 * 60 + 0; // 18:00

	// peas / bohnen
	all_timers[3].ventile = 2;
	all_timers[3].minutes = 15;
	all_timers[3].minute_at_day = 7 * 60 + 1; // 7:01

	//all_timers[4].ventile = 2;
	//all_timers[4].minutes = 5;
	//all_timers[4].minute_at_day = 12 * 60;
	
	//all_timers[5].ventile = 2;
	//all_timers[5].minutes = 5;
	//all_timers[5].minute_at_day = 15 * 60;
	
	all_timers[6].ventile = 2;
	all_timers[6].minutes = 10;
	all_timers[6].minute_at_day = 18 * 60 + 1; // 18:01
	
	// erdbeeren
	all_timers[7].ventile = 1;
	all_timers[7].minutes = 20;
	all_timers[7].minute_at_day = 7 * 60 + 10; // 07:10

	all_timers[8].ventile = 1;
	all_timers[8].minute_at_day = 18 * 60 + 2; // 18:02
	all_timers[8].minutes = 10;



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


struct brightness {
	
	uint16_t values[5]{ 0, 0, 0, 0, 0 };
	uint32_t repeat{ 0 };
	
	brightness(){};
	
	brightness& operator = (const brightness& another){
		for (uint8_t i = 0; i < 5; ++i)
		values[i] = another.values[i];
		repeat = another.repeat;
		return *this;
	}
};

template<uint8_t _Buffer_Size>
struct brightness_buffer {
	static_assert(_Buffer_Size > 0, "Size 0 buffer not allowed.");
	brightness _buffer[_Buffer_Size]{};
	
	uint8_t next_read{0};
	uint8_t next_write{0};
	uint8_t filled{0};
	
	private:
	
	inline bool unsafe_empty() const {
		return filled == 0;
	}
	inline bool unsafe_full() const {
		return filled == _Buffer_Size;
	}
	public:
	bool empty() const {
		auto lock = fsl::hw::simple_atomic();
		return unsafe_empty();
	}
	bool full() const {
		fsl::hw::simple_atomic lock;
		return unsafe_full();
	}
	
	bool write(const brightness& element){
		fsl::hw::simple_atomic lock;
		if (unsafe_full()) return false;
		_buffer[next_write++] = element;
		++filled;
		next_write %= _Buffer_Size;
		return true;
	}
	/*
	bool read(brightness& destination){
	auto lock = fsl::hw::simple_atomic();
	if (unsafe_empty()) return false;
	destination = _buffer[next_read];
	if (--repeat )
	--filled;
	++next_read;
	next_read %= _Buffer_Size;
	
	return true;
	}
	*/
	bool unsafe_uninterrupted_read_ptr(const brightness*& ptr){
		if (unsafe_empty()) return false;
		ptr = &_buffer[next_read];
		if (_buffer[next_read].repeat == 0){
			--filled;
			++next_read;
			next_read %= _Buffer_Size;
		}
		else {
			--_buffer[next_read].repeat;
		}
		return true;
	}
	
};

struct bulb
{
	static const uint8_t fast_map[32];
	
	static brightness_buffer<10> buffer;
	
};

brightness_buffer<10> bulb::buffer;

const uint8_t bulb::fast_map[32]{
	0,
	1,
	0,
	2,
	0,
	1,
	0,
	3,
	0,
	1,
	0,
	2,
	0,
	1,
	0,
	4,
	0,
	1,
	0,
	2,
	0,
	1,
	0,
	3,
	0,
	1,
	0,
	2,
	0,
	1,
	0,
	0
};

void update_arc(){
	static uint8_t circle_position{0};
	circle_position +=1;
	circle_position %= 32;
	
	const brightness* br;
	bool success = bulb::buffer.unsafe_uninterrupted_read_ptr(br);
	if (!success) return;
	
	uint16_t bulb_vector { br->values[bulb::fast_map[circle_position]] };
	
	const uint8_t on_D { bulb_vector & 0xFF };
	const uint8_t on_B { 0b1 & (bulb_vector >> 8) };
	
	//DDRD = on_D;
	PORTD = on_D;
	
	//DDRB = (DDRB & ~0b1) | on_B;
	PORTB = (PORTB & ~0b1) | on_B;
}

void set_br(brightness& b, uint16_t bulb_mask, uint8_t br){
	for (uint8_t i = 0; i < 5; ++i){
		b.values[i] |= (br & (1 << 4-i)) ? bulb_mask : 0;
	}
}

void enlight_up(){
	// all off
	while (bulb::buffer.full());
	brightness all_off;
	all_off.repeat = 16000;
	bulb::buffer.write(all_off);
	
	uint16_t mask[5]{
		0b100000001,
		0b010000010,
		0b001000100,
		0b000101000,
		0b000010000
	};
	
	uint8_t br_old{0};
	uint8_t br_new{0};
	uint8_t br_diff{1};
	uint8_t change_br_diff{4};
	
	while (true) {
		// calc next brightness
		br_old = br_new;
		if (br_new == 28){
			br_new = 31;
		}
		else {
			br_new += br_diff;
			--change_br_diff;
			if (!change_br_diff){
				change_br_diff = 4;
				br_diff*=2;
			}
		}
		for (uint8_t height = 0; height < 5; ++height){
			brightness x;
			x.repeat = 1000;
			for(uint8_t i = 0; i < height; ++i){
				set_br(x,mask[i],br_new);
			}
			for(uint8_t i = height; i < 5; ++i){
				set_br(x,mask[i],br_old);
			}
			while (bulb::buffer.full());
			bulb::buffer.write(x);
		}
		
		if (br_new == 31)
		goto part_b;
	}
	part_b:
	while (br_new > 0) {
		brightness x;
		set_br(x,0b1'1111'1111,br_new);
		x.repeat = 1000 + br_new * 100 + 3000 * (br_new < 6);
		while (bulb::buffer.full());
		bulb::buffer.write(x);
		--br_new;
	}
}

using time_type = uint16_t;

constexpr uint8_t TICKER_MAX{ 34 };
constexpr time_type DIVIDE_TIME_LONGATION{ 22 };
constexpr time_type DIVIDE{ DIVIDE_TIME_LONGATION * 5};
constexpr uint8_t br_table_max_index{ 8 };

template <uint8_t I>
constexpr time_type n_th_DIVIDE_TIME_LONGATION(){
	return I * DIVIDE_TIME_LONGATION;
}

constexpr time_type T_MAX{ br_table_max_index * DIVIDE - n_th_DIVIDE_TIME_LONGATION<4>() };

time_type func(const time_type& value){
	return value * value;
}

const uint8_t br_table[br_table_max_index]{
	0,
	//1,
	1,
	3,
	//3,
	7,
	10,
	16,
	//16,
	22,
	35
};


struct bulb_buffer{
	using buffer_size_type = uint8_t;
	static constexpr buffer_size_type BUFFER_SIZE{ 255 };
	using reg_pair = uint8_t[2];
	using buffer = reg_pair[BUFFER_SIZE];
	
	buffer _array;

	reg_pair* const begin{ &_array[0] };
	reg_pair* const end{ begin + BUFFER_SIZE };

	reg_pair* next_read{ begin };
	reg_pair* next_write{ begin };
	
	bulb_buffer(){
		for(auto iter = begin; iter != end; ++iter){
			(*iter)[0] = 0;
			(*iter)[1] = 0;
		}
	}
	
	
};

bulb_buffer global_bulb_buffer;

void update_bulbs(){
	PORTB = (PORTB & ~0b1) | (0b1 & (*global_bulb_buffer.next_read)[1]);
	PORTD = (*global_bulb_buffer.next_read)[0];
	++global_bulb_buffer.next_read;
	if (global_bulb_buffer.next_read == global_bulb_buffer.end){
		global_bulb_buffer.next_read = global_bulb_buffer.begin;
	}
}

struct pattern {
	static constexpr uint16_t LEVEL_0{ 0b1'0000'0001 };
	static constexpr uint16_t LEVEL_1{ 0b0'1000'0010 };
	static constexpr uint16_t LEVEL_2{ 0b0'0100'0100 };
	static constexpr uint16_t LEVEL_3{ 0b0'0010'1000 };
	static constexpr uint16_t LEVEL_4{ 0b0'0001'0000 };
};


struct brightness_config
{
	using divider_type = uint8_t;
	using divider_array = divider_type[9];
	static constexpr divider_type divide_size = 50;
	
	using bulb_position = uint8_t[9];
};

void add_buffer(brightness_config::divider_type divide_size, const brightness_config::divider_array& bulb_brightness_ascending, const brightness_config::bulb_position& positions){
	uint16_t reg{ 0x1FF };
	uint8_t next_bulb{ 0 };
	for(brightness_config::divider_type i = 0; i < divide_size; ++i){
		while (next_bulb < 9 && i >= bulb_brightness_ascending[next_bulb]){
			reg &= ~(static_cast<uint16_t>(1) << positions[next_bulb]);
			++next_bulb;
		}
		// write reg to buffer if not buffer full.
	}
}

void fill_buffer_blink_001(){
	while(true){
		uint8_t ticker{ 0 };
		time_type time{ 0 };
		

		while (time != T_MAX){
			ticker += 7;
			ticker %= TICKER_MAX;
			time += (ticker == 0);
			//time %= T_MAX;
			uint16_t bulbs =
			(ticker < br_table[(time + n_th_DIVIDE_TIME_LONGATION<4>()) / DIVIDE ]) * pattern::LEVEL_0 |
			(ticker < br_table[(time + n_th_DIVIDE_TIME_LONGATION<3>()) / DIVIDE ]) * pattern::LEVEL_1 |
			(ticker < br_table[(time + n_th_DIVIDE_TIME_LONGATION<2>()) / DIVIDE ]) * pattern::LEVEL_2 |
			(ticker < br_table[(time + n_th_DIVIDE_TIME_LONGATION<1>()) / DIVIDE ]) * pattern::LEVEL_3 |
			(ticker < br_table[(time + n_th_DIVIDE_TIME_LONGATION<0>()) / DIVIDE ]) * pattern::LEVEL_4;
			PORTD = bulbs & 0xFF;
			PORTB = (PORTB & ~0b1) | (0b1 & (bulbs >> 8));
			//while (time == 30){}
		}
		PORTD = 0xFF;
		PORTB |= 0b1;
		for (uint32_t wait=0; wait < 100000ull; ++wait) /* wait */;
		for (uint8_t br = 255; br != 0; --br){
			for (uint16_t i = 0; i < 5; ++i){
				for(uint8_t pwm = 0; pwm < 255; ++pwm){
					if (pwm < br){
						PORTD = 0xFF;
						PORTB |= 0b1;
						} else {
						PORTD = 0;
						PORTB &= ~0b1;
					}
				}
			}
		}
		PORTD = 0;
		PORTB &= ~0b1;
		for (uint32_t wait=0; wait < 100000ull; ++wait) /* wait */;
	}
}


int main(void)
{
	auto& all{ bulb::buffer };
	
	pin_init();
	
	//super_init_timers();
	//the_clock.set();
	
	// brightness callibration
	
	reset_timer_1();
	activate_timer1_compare_match_interrupt_A(50, update_bulbs, true);
	
	start_timer1_prescaler_1();
	
	while (true)
	{
		fill_buffer_blink_001();
	}
}