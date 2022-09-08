#include "config.h"

#ifdef PUMP_RELAY_MAYSON



// led auto
#define PIN_D0 16
// led system
#define PIN_D1 5
// led wifi
#define PIN_D2 4

// button
#define PIN_D3 0
// button
#define PIN_D4 2

// led: do not use it prevents from boot and flashing when pulled down via led.
#define PIN_D5 14
// led manual
#define PIN_D6 12
// led pump relay
#define PIN_D7 13



#define GPIO_INPUT_LANES ((1ULL<<PIN_D3) | (1ULL<<PIN_D4)) 

#define GPIO_OUTPUT_LANES ((1ULL<<PIN_D1) | (1ULL<<PIN_D2) | (1ULL<<PIN_D0) | (1ULL<<PIN_D6) | (1ULL<<PIN_D7))



#endif // PUMP_RELAY_MAYSON


