
//#include "timer0.h"
//#include "display.h"
//#include "input.h"
//#include "time_utilities.h"
//#include "queue.h"
//#include "pump.h"
//#include "ui_utility.h"
#include <avr/io.h>
#include <avr/eeprom.h>


void pin_init(){

	DDRD = 0b11111111; // out valves
	PORTD = 0b00000000; // all valves closed

	DDRB  = 0b00011000; // communication to master : X X X : CLOCK : DATA : SYNC : CLOCK : DATA
	PORTB = 0b00000111; // pull-up resistors for input lanes.
}


bool is_sync_input_lane_active(){
	return ! (PINB & 0b100);
}

bool is_clock_input_lane_active(){
	return ! (PINB & 0b010);
}

bool is_data_input_lane_active(){
	return ! (PINB & 0b001);
}

void set_clock_output_lane(bool b){
	PORTB = (PORTB & (~0b00010000)) | (b ? 0b00010000 : 0b0);
}

void set_data_output_lane(bool b){
	PORTB = (PORTB & (~0b00001000)) | (b ? 0b00001000 : 0b0);
}

void esp_comm_sync(){

	again_sync:
	while (!is_sync_input_lane_active() || is_data_input_lane_active() || is_clock_input_lane_active()){
		//wait for sync begin
	}
	set_data_output_lane(false);
	set_clock_output_lane(false);
	while (!is_clock_input_lane_active())
	{
		// wait for clock up
		if (!is_sync_input_lane_active() || is_data_input_lane_active()){
			goto again_sync;
		}
	}
	set_clock_output_lane(true);
	while (is_clock_input_lane_active())
	{
		// wait for clock down
		if (is_data_input_lane_active()){
			goto again_sync;
		}
	}
	if (is_sync_input_lane_active()) {
		goto again_sync;
	}
	set_clock_output_lane(false);
	return;
}

/*
enum class
RECEIVE_PHASE : char {
	READ_EEPROM_SUCCESS = 0,
	OPCODE_SET_VALVES = 1,
};
*/

/**
@brief Reads opcode from esp_comm_line.

@param opcode will be replaced by opcode read or bits read so far until canceled.
@return Returns false if re-sync initiated, aborted reading opcode. Returns true if reading opcode was read successfully.
*/


namespace fsl {
	
	template <class T> struct remove_reference;
	template <class T> struct remove_reference			{ using type = T; };
	template <class T> struct remove_reference<T&>		{ using type = T; };
	template <class T> struct remove_reference<T&&>		{ using type = T; };
	
	template <class T>
	using remove_reference_t = typename remove_reference<T>::type;
	
}


template<class Integer_Type>
bool try_read_bits_from_esp(uint8_t count_bits, Integer_Type& read_buffer){
	read_buffer = 0;
	int pos = 0;
	while (count_bits)
	{
		// read one bit:
		while(!is_clock_input_lane_active()){
			if (is_sync_input_lane_active()){
				return false;
			}
		}
		bool bit = is_data_input_lane_active();
		--count_bits;
		//read_buffer = bit;
		read_buffer |=  (static_cast<fsl::remove_reference_t<decltype(read_buffer)>>(1) << pos) * bit; // data are received LSB first!
		++pos;
		set_clock_output_lane(true);
		while (is_clock_input_lane_active()){
			if (is_sync_input_lane_active()){
				return false;
			}
		}
		set_clock_output_lane(false);
		
	}
	return true;
}

int main(void)
{
	
	pin_init();
	
	again_sync:
	esp_comm_sync();
	// sync done.
	
	while (true){
		uint8_t opcode = 0;
		if (!try_read_bits_from_esp(4, opcode)){
			goto again_sync;
		}

		if (opcode == 0b0001){
			uint8_t valves = 0;
			if (!try_read_bits_from_esp(8,valves)){
				goto again_sync;
			}
			PORTD = valves;
			continue;
		}
		
		goto again_sync; // no valid opcode
		
	}
	
	PORTD = 0;

	while (true)
	{
	}
	
}