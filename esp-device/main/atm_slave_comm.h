#pragma once

#include "select_device.h"

#include "freertos/FreeRTOS.h"

#ifdef VALVE_SERVER_JAMES

bool send_bits_u8(uint8_t data, uint8_t count_bits, TickType_t t0, TickType_t timeout_difference);
bool read_bits_u16(uint16_t* data, uint8_t count_bits, TickType_t t0, TickType_t timeout_difference);

#endif // VALVE_SERVER_JAMES


