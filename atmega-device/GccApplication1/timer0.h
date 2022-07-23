/*
 * timer0.h
 *
 * Created: 29.07.2020 16:40:19
 *  Author: F-NET-ADMIN
 */ 


#ifndef TIMER0_H_
#define TIMER0_H_

#include <avr/interrupt.h>

using procedure = void (*)();

extern procedure TCNT0_COMPARE_A;

inline void reset_timer_0(){ // 8-bit timer
	TCCR0B &= 0b11111000;
	TCNT0 = 0;
}

inline void start_timer_prescaler_64(){	TCCR0B |= 0b00000011;	}

inline void start_timer_prescaler_1024(){	TCCR0B |= 0b00000101;	}

inline void activate_timer0_compare_match_interrupt_A(uint8_t compareTo, procedure interrupt, bool enable_ctc_on_compare_A){ // set compare value and interrupt routine.
	OCR0A = compareTo;
	TCNT0_COMPARE_A = interrupt;
	TIMSK0 |= 1 << OCIE0A; // enable interrupt;
	sei(); // global interrupt enable;
	if (enable_ctc_on_compare_A){
		TCCR0A |= 1 << WGM01;
	}
}

inline void disable_timer0_compare_match_interrupt(){	TIMSK0 &= ~(1 << OCIE0A);	}

extern procedure TCNT1_COMPARE_A;

inline void reset_timer_1(){
	TCCR1B &= 0b11111000;
	TCNT1 = 0;
}

inline void start_timer1_prescaler_64(){	TCCR1B |= 0b00000011;	}

inline void start_timer1_prescaler_1024(){	TCCR1B |= 0b00000101;	}

inline void activate_timer1_compare_match_interrupt_A(uint16_t compareTo, procedure interrupt, bool enable_ctc_on_compare_A){ // set compare value and interrupt routine.
	OCR1A = compareTo;
	TCNT1_COMPARE_A = interrupt;
	TIMSK1 |= 1 << OCIE1A; // enable interrupt;
	sei(); // global interrupt enable;
	if (enable_ctc_on_compare_A){
		TCCR1A |= 1 << WGM11;
	}
}

inline void disable_timer1_compare_match_interrupt(){	TIMSK1 &= ~(1 << OCIE1A);	}


#endif /* TIMER0_H_ */





#if false

class send_sequence {
	public:
	volatile uint8_t* volatile begin;
	volatile uint32_t bit_size;
	volatile uint32_t next;
	volatile bool ready = false;
	
	void reset_for_fill(volatile uint8_t* buffer, uint32_t max_size_bits){
		begin = buffer;
		bit_size = max_size_bits;
		next = 0;
		ready = false;
	}
	
	bool add_bit(bool bit){
		if (next == bit_size) return false;
		if (bit) begin[next/8] = (begin[next/8] & ~(1 << (next % 8))) | (1 << (next % 8));
		++next;
		return true;
	}
	
	void finish_fill(){
		bit_size = next;
		next = 0;
	}
	
	bool add_bits(uint64_t sequence, uint8_t count){
		bool success;
		for (uint8_t i = 0; i < count; ++i){
			success = add_bit(sequence & (uint64_t(1) << i));
		}
		return success;
	}
};

static send_sequence wireless_sequence;
static volatile uint8_t send_bit_buffer[16];





void turn_on_sender(bool on = true){
	_delay_ms(0.5);
	PORTC = (PORTC & (~0b00000010)) | (on ? 0b10 : 0);
	if (on){ _delay_ms(50); } // does the sender need some start-up time??? -> needs to be tested.
}

inline void turn_off_sender(){
	turn_on_sender(false);
}

void set_sender_bit(bool x){
	PORTC = (PORTC & (~0b00000001)) | (x ? 0b1 : 0);
}


void sequence_sender(send_sequence& s){ // make it as method.
	if (s.next == s.bit_size) {
		s.ready = true;
		return;
	}
	uint16_t byte = s.next / 8;
	uint8_t bit = s.next % 8;
	set_sender_bit(s.begin[byte] & (uint64_t(1) << bit));
	++s.next;
}

void send_interrupt_routine(){
	if (wireless_sequence.ready){
		disable_timer0_compare_match_interrupt();
		turn_off_sender();
	}
	sequence_sender(wireless_sequence);
}

void start_send_via_interrupts(){
	reset_timer_0();
	activate_timer0_compare_match_interrupt_A(128,send_interrupt_routine, true);
	turn_on_sender();
	start_timer_prescaler_64();
}


inline void raw_send_output_sequence(uint64_t sequence, uint8_t count){
	for (uint8_t i = 0; i < count; ++i){
		_delay_ms(0.5);
		set_sender_bit(sequence & (uint64_t(1) << i));
		_delay_ms(0.5);
	}
}

/*
@details input: 4 bit (lower nibble is considered). return: lower 10 bits are of interest
*/
uint16_t cyclic_code_encode(uint8_t nibble){
	uint16_t result{ 0 };
	for (uint8_t i = 0; i < 4; ++i){
		result ^= (GENERATOR << i) * ((nibble & (uint8_t(1) << i)) != 0);
	}
	result ^= OFFSET;
	return result;
}

/*
@details input considers 10 lower bits. returns 4 bits decoded (lower bits).
sets error to true if error, otherwise error is untouched
*/
uint8_t cyclic_code_decode(uint16_t encoded_nibble, bool& error){
	uint16_t result{ 0 };
	encoded_nibble ^= OFFSET;
	for (uint8_t ii = 0; ii < 4; ++ii){
		_Pragma("GCC diagnostic push")
		_Pragma("GCC diagnostic ignored \"-Wnarrowing\"")
		const uint8_t i{ 3 - ii };
		_Pragma("GCC diagnostic pop")
		const uint16_t mask_for_considered_bit{ uint16_t(1) << (6 + i) };
		const bool recognized{ encoded_nibble & mask_for_considered_bit };
		encoded_nibble ^= (GENERATOR << i) * recognized;
		result |= (uint8_t(1) << i) * recognized;
	}
	if (encoded_nibble) {
		error = true;
		return 0;
	}
	return result;
}


/**
The first 25 bits have to be sent. convertes 8 bit message into final sendable bit sequence.
*/
uint32_t prepare_send_byte(uint8_t x){
	const uint32_t send_sequence_low{ cyclic_code_encode(x) };
	const uint32_t send_sequence_high{ cyclic_code_encode(x >> 4) };
	const uint32_t* sequences[2] {&send_sequence_low, &send_sequence_high};
	uint32_t glued{ 0 };
	uint8_t glue_index{ 0 };
	uint8_t pad_offset{ 0 };
	for (uint8_t sequence = 0; sequence < 2; ++sequence){
		const uint32_t& s = *sequences[sequence];
		for (uint8_t i = 0; i < 10; ++i){
			glued |= (uint32_t(1) << (pad_offset + glue_index++)) * ((s & (uint32_t(1) << i)) != 0);
			if (!(glue_index % 4)){
				pad_offset++; // send a "0" bit
			}
		}
	}
	return glued;
}

/**
decodes lower 25 bits into 8 bits message
*/
uint8_t decode_received_byte_block(uint32_t encoded, bool& error){
	uint32_t without_padding{ 0 };
	{ // remove padding
		uint8_t pad_offset{ 0 };
		for (uint8_t i = 0; i < 20; ++i){
			without_padding |=  (uint32_t(1) << i) * ((encoded & (uint32_t(1) << (i + pad_offset))) != 0);
			if (!((i + 1) % 4)){
				pad_offset++; // send a "0" bit
			}
		}
	}
	uint8_t result{ 0 };
	result |= cyclic_code_decode(without_padding & 0b1111111111, error); // lower nibble
	result |= cyclic_code_decode((without_padding & 0b11111111110000000000) >> 10, error) << 4;// higher nibble
	return result;
}



void send(const uint32_t& data){
	uint32_t sequences[4];
	for(uint8_t i = 0; i < 4; ++i){
		uint8_t byte = static_cast<uint8_t>((data >> 8*i) & uint32_t(0b11111111));
		sequences[i] = prepare_send_byte(byte);
	}
	wireless_sequence.reset_for_fill(send_bit_buffer,16*8);
	wireless_sequence.add_bits(SYNC, 16);
	for(uint8_t i = 0; i < 4; ++i){
		wireless_sequence.add_bits(sequences[i], 25);
	}
	wireless_sequence.finish_fill();
	disp(5);
	start_send_via_interrupts();
}

class receiver_buffer {
	public:
	volatile uint8_t recent_bits;
	volatile bool current_bit;
	volatile uint16_t global_count_captures;
	static constexpr uint8_t SWITCHES = 10;
	volatile uint16_t switch_times[SWITCHES];
	volatile uint8_t switch_index_next;
	volatile uint8_t start_found;
	volatile uint8_t* volatile  begin; // read data.
	volatile uint32_t bit_size;
	volatile uint32_t next_bit;
	volatile bool ready;
	
	void search_start_mode(bool bit){
		disp(6);
		++global_count_captures;
		recent_bits = recent_bits << 1 | static_cast<uint8_t>(bit);
		// recalculate current bit:
		uint8_t sum = 0;
		for (uint8_t i = 0; i < 8; ++i) sum += !(recent_bits & (1 << i));
		bool old_bit = current_bit;
		if (sum < 4) current_bit = true;
		if (sum > 4) current_bit = false;
		// check bit flip:
		if (old_bit != current_bit) switch_times[switch_index_next] = global_count_captures;
		// check syn captured in switch times:
		auto near = [](uint16_t left, uint16_t right, uint16_t epsilon){
			return left + epsilon > right && right + epsilon > left;
		};
		disp(1);
		if (
		(current_bit == false) &&
		near(switch_times[switch_index_next], switch_times[(switch_index_next + SWITCHES - 1) % SWITCHES] + 6*8,3) &&
		near(switch_times[(switch_index_next + SWITCHES - 1) % SWITCHES], switch_times[(switch_index_next + SWITCHES - 2) % SWITCHES]+8,3) &&
		near(switch_times[(switch_index_next + SWITCHES - 2) % SWITCHES], switch_times[(switch_index_next + SWITCHES - 3) % SWITCHES]+8,3) &&
		near(switch_times[(switch_index_next + SWITCHES - 3) % SWITCHES], switch_times[(switch_index_next + SWITCHES - 4) % SWITCHES]+8,3) &&
		near(switch_times[(switch_index_next + SWITCHES - 4) % SWITCHES], switch_times[(switch_index_next + SWITCHES - 5) % SWITCHES]+8,3) &&
		near(switch_times[(switch_index_next + SWITCHES - 5) % SWITCHES], switch_times[(switch_index_next + SWITCHES - 6) % SWITCHES]+8,3) &&
		near(switch_times[(switch_index_next + SWITCHES - 6) % SWITCHES], switch_times[(switch_index_next + SWITCHES - 7) % SWITCHES]+8,3) &&
		near(switch_times[(switch_index_next + SWITCHES - 7) % SWITCHES], switch_times[(switch_index_next + SWITCHES - 8) % SWITCHES]+8,3) &&
		near(switch_times[(switch_index_next + SWITCHES - 8) % SWITCHES], switch_times[(switch_index_next + SWITCHES - 9) % SWITCHES]+8,3)
		) {
			start_found = 3;
		}
		switch_index_next++;
		switch_index_next %= SWITCHES;
		// check if Interrupt timer is over-rolled here -> use smaller prescaler and check if value is above threshold? -> use comparematch b?
	}
	
	void capture_data(bool bit){
		disp(7);
		++global_count_captures;
		global_count_captures %= 8;
		recent_bits = recent_bits << 1 | static_cast<uint8_t>(bit);
		if (!global_count_captures){
			// read bit
			uint8_t sum = 0;
			for (uint8_t i = 1; i < 7; ++i) sum += !(recent_bits & (1 << i));
			const bool captured_bit = sum > 3;
			if (next_bit == bit_size){
				ready = true;
				return;
			}
			begin[next_bit / 8] = (begin[next_bit / 8] & ~(1 << (next_bit % 8))) | ((1 << (next_bit % 8)) * captured_bit);
		}
	}
	
	void append(bool bit){
		led_on_off(bit);
		if (!start_found) return search_start_mode(bit);
		if (start_found == 1) return capture_data(bit);
		global_count_captures = 0;
		recent_bits = 0;
		--start_found;
	}
	
	void reset(volatile uint8_t* buffer, uint32_t bit_size){
		begin = buffer;
		this->bit_size = bit_size;
		
		recent_bits = 0;
		current_bit = 0;
		global_count_captures = 0;
		for (uint8_t i = 0; i < SWITCHES; ++i){
			switch_times[i] = 0xFFFF;
		}
		switch_index_next = 0;
		next_bit = 0;
		start_found = false;
		ready = false;
	}
};

static receiver_buffer rbuf;

static volatile uint8_t buf_array[13];

void sync_capture_receiver(){
	rbuf.append(get_receiver_bit());
	if (rbuf.ready) {
		disable_timer0_compare_match_interrupt();
		reset_timer_0();
	}
}

void start_receive_via_interrupts(){
	reset_timer_0();
	activate_timer0_compare_match_interrupt_A(16,sync_capture_receiver, true);
	rbuf.reset(buf_array,100);
	start_timer_prescaler_64();
}

static constexpr uint32_t TEST_MESSAGE = 0b11001100101010100001110011101011;

#define ON_SENDER true
#define ON_RECEIVER (!ON_SENDER)

bool get_test_button(){
	return !(PIND & 0b00000001);
}

static volatile uint8_t count_led;

void toggle_led(){
	if (!count_led){
		disable_timer0_compare_match_interrupt();
		reset_timer_0();
		return;
	}
	bool led = PORTD & 0b10;
	PORTD = (PORTD & 0b11111101) | (0b10 * !led);
	--count_led;
}

uint32_t fetch_received_message(){
	bool error = false;
	uint32_t messages[4]{0,0,0,0};
	uint32_t result = 0;
	for (uint32_t i = 0; i < rbuf.bit_size; ++i){
		messages[i/25] |= (1 << (i%25)) * !!(rbuf.begin[i/8] & (1 << (i%8)));
	}
	for (uint8_t i = 0; i < 4; ++i) result |= decode_received_byte_block(messages[i], error) << (8*i);
	return error ? 0xFFFFFFFF : result;
}

#endif