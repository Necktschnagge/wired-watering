
#define F_CPU 1000000UL

#include <avr/io.h>


void pin_init(){

	DDRD = 0b11111111; // out valves
	PORTD = 0b00000000; // all valves closed

	DDRB  = 0b00011000; // communication to master : X X X : CLOCK : DATA : SYNC : CLOCK : DATA
	PORTB = 0b00000111; // pull-up resistors for input lanes.
	
	// connect AREF port to VCC
	DDRC  = 0b00000000; // XXXX XXX[ADC pressure sensor input ADC0]
	PORTC = 0b00000000; // no pull-up for sensor input
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


namespace fsl {
	
	template <class T> struct remove_reference;
	template <class T> struct remove_reference			{ using type = T; };
	template <class T> struct remove_reference<T&>		{ using type = T; };
	template <class T> struct remove_reference<T&&>		{ using type = T; };
	
	template <class T>
	using remove_reference_t = typename remove_reference<T>::type;
	
}

bool read_10bit_ADC_pressure_sensor(uint16_t& measure_result){
	bool successfully{ true };
	
	PRR &= ~0b00000001;		// Power Reduction Register : Power Reduction ADC : shuts down the ADC when high

	ADCSRA = 0b10010100; /*
	7: / ADC Control and Status Register A : ADC Enable : 1 == ADC enabled
	6: Start Conversion
	5: Auto Trigger enable
	4: interrupt flag, enabled when conversion completed, disabled by writing a 1, must be disabled before starting a new conversion
	3: interrupt enable for ADC completion
	2:0 clock speed pre-scaler (0b100 is pre-scaler 16)
	*/

	ADMUX = 0b00000000; /*
	7:6		use AREF as reference
	5:		result is right adjusted
	4:		reserved
	3:0		select input: ADC0
	*/
	

	ADCSRA |= 0b01000000;	// ADC Control and Status Register A : ADC Start Conversion : set to 1 to start each conversion
	// connect AREF and AVCC to 5V	

	uint64_t timeout = F_CPU / 4;
	while((--timeout>0) && !(ADCSRA & 0b10000)){
		/* busy waiting */
	}
	measure_result = ADCW; // read result
	if (timeout == 0) {
		measure_result = 0xFFFF;
		successfully = false;
	}
	
	ADCSRA &= ~0b10000000; // ADC Control and Status Register A : ADC Enable : 1 == ADC enabled -> disable ADC

	// maybe (re-)enable power reduction. PRR register -> no dont, read ADC "Overview" in docs to know why...
	
	return successfully;
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

template<class Integer_Type>
bool try_send_bits_to_esp(uint8_t count_bits, Integer_Type send_bit_sequence){
	while (count_bits)
	{
		// send one bit:
		while(!is_clock_input_lane_active()){ // wait for receive-ready signal
			if (is_sync_input_lane_active()){
				return false;
			}
		}
		
		set_data_output_lane(send_bit_sequence & 0b1); // send LSB first
		send_bit_sequence >>= 1; // shift
		--count_bits;
		
		set_clock_output_lane(true); // data_output is set
		while (is_clock_input_lane_active()){ // wait for receive confirmation
			if (is_sync_input_lane_active()){
				return false;
			}
		}
		// got receive confirmation
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

		if (opcode == 0b0001){ // set-valves
			uint8_t valves = 0;
			if (!try_read_bits_from_esp(8,valves)){
				goto again_sync;
			}
			PORTD = valves;
			continue;
		}

		if (opcode == 0b0010){ // read-pressure
			uint16_t pressure = 0;
			bool success = read_10bit_ADC_pressure_sensor(pressure);
			if (success){
				pressure &= 0x0FFF;
			}
			if (!success){
				pressure = 0x8000;
			}
			if (!try_send_bits_to_esp(16, pressure)){
				goto again_sync;
			}
			continue;
		}
		
		goto again_sync; // no valid opcode
	}
	
	PORTD = 0;

	while (true)
	{
	}
}