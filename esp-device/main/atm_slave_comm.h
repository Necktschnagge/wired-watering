
#include "select_device.h"

//#include "myutils.h"

#include "freertos/task.h"

//!!!!!!!!!!!!!!!!!!!!!!!!!!! remove as many as possible

//! #include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "freertos/event_groups.h"
//#include "esp_system.h"
//#include "esp_log.h"
//#include "esp_netif.h"
//#include "esp_event.h"
//#include "esp_wifi.h"
//#include "protocol_examples_common.h"
//#include "nvs.h"
//#include "nvs_flash.h"
//#include <esp_http_server.h>
//#include "driver/gpio.h"
//!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "stdint.h" // uint8_t


#ifdef VALVE_SERVER_JAMES

bool send_bits_u8(uint8_t data, uint8_t count_bits, TickType_t t0, TickType_t timeout_difference);

#endif // VALVE_SERVER_JAMES
