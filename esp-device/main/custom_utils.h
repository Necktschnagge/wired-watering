#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define wait_to_not_be_busy vTaskDelay(1) /// otherwise we block other tasks and watchdog kills the whole system -> reboot(?)

