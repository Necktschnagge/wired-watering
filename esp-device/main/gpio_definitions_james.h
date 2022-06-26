#include "config.h"

#ifdef VALVE_SERVER_JAMES



// ESP_TO_ATM_SYNC
#define PIN_D1 5
#define ESP_TO_ATM_SYNC PIN_D1

// ESP_TO_ATM_CLOCK
#define PIN_D2 4
#define ESP_TO_ATM_CLOCK PIN_D2

// ESP_TO_ATM_DATA
#define PIN_D7 13
#define ESP_TO_ATM_DATA PIN_D7

// ATM_TO_ESP_DATA // blue LED on wifi board is on if pulled low.
#define PIN_D4 2
#define ATM_TO_ESP_DATA PIN_D4

// ATM_TO_ESP_CLOCK
#define PIN_D3 0
#define ATM_TO_ESP_CLOCK PIN_D3



#define GPIO_INPUT_LANES ((1ULL<<ATM_TO_ESP_DATA) | (1ULL<<ATM_TO_ESP_CLOCK))

#define GPIO_OUTPUT_LANES ((1ULL<<ESP_TO_ATM_DATA) | (1ULL<<ESP_TO_ATM_CLOCK) | (1ULL<<ESP_TO_ATM_SYNC))



#endif // VALVE_SERVER_JAMES


