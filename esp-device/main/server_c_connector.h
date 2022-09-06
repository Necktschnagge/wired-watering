#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

	int c_for_get_set_valve_answer(char* first_char, int length);
	int c_for_get_relay_answer(char* first_char, int length);
	int c_for_get_pressure_answer(char* first_char, int length, uint16_t raw_pressure);

#ifdef __cplusplus
}
#endif

