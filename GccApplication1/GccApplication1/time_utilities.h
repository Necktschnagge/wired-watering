/*
* time_utilities.h
*
* Created: 02.08.2020 13:11:45
*  Author: F-NET-ADMIN
*/


#ifndef TIME_UTILITIES_H_
#define TIME_UTILITIES_H_

#include "f_interrupt.h"

/* CPU Frequency: ~ 1MHz */
// #define F_CPU 1000000 clock ticked 23:10 in 24h (too slow)

// f_n+1 = f_n * (time_the_mc_clock_ticked / measure_interval)
// f_0 = 1000000Hz // best guess
// f_1 = f_0 * (1390min / 1440min) = ~ 965278Hz // 24h - messure and correction
// last_modified: 02.08.2020 10:30
// f_2 = f_1 * (8835min / 8865min) = ~ 962011Hz // 6 days, 3 hours, 45 min measure interval.
// last_modified: 08.08.2020 14:30
//

#define F_CPU 962011
static constexpr uint32_t CPU_FREQUENCY{ F_CPU };

#include <util/delay.h>


inline void sleep(uint32_t ms){
	for (uint32_t i = 0; i < ms/10; ++i) _delay_ms(10);
}

class human_clock {
	public:
	using time_type = uint64_t;
	
	static constexpr time_type SECOND{ 1 };
	static constexpr uint64_t MINUTE{ 60 };
	static constexpr uint64_t HOUR{ 60 * MINUTE };
	static constexpr uint64_t DAY{ 24 * HOUR };
	
	private:
	volatile uint64_t human_clock_seconds{ 0 };
	volatile uint64_t clock_offsets{ 0 };
	
	inline void inc(){
		clock_offsets += 32ul * 1024;
		bool tick = clock_offsets >= CPU_FREQUENCY;
		if (tick) ++human_clock_seconds;
		clock_offsets %= CPU_FREQUENCY;
	}
	
	static void inc_human_clock(){ return instance().inc(); }

	public:
	inline static human_clock& instance(){
		static human_clock the_clock;
		return the_clock;
	}

	void set();

	inline uint64_t now() const {
		fsl::hw::stupid_atomic atomic;
		auto copy = human_clock_seconds;
		return copy;
	}

	uint16_t minute_of_day() const {	return static_cast<uint16_t>((now() % DAY) / MINUTE);	}
};


#endif /* TIME_UTILITIES_H_ */