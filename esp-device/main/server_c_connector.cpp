#include "server_c_connector.h"
#include "server.h"

#ifdef __cplusplus
extern "C" {
#endif

    // Inside this "extern C" block, I can implement functions in C++, which will externally 
    //   appear as C functions (which means that the function IDs will be their names, unlike
    //   the regular C++ behavior, which allows defining multiple functions with the same name
    //   (overloading) and hence uses function signature hashing to enforce unique IDs),

    int c_for_get_set_valve_answer(char* first_char, int length) {
        int result = get_server_answer_string(first_char, length);
        return result;
    }

    int c_for_get_relay_answer(char* first_char, int length) {
        int result = get_relay_answer_string(first_char, length);
        return result;
    }

    int c_for_get_pressure_answer(char* first_char, int length, uint16_t raw_pressure) {
        int result = get_pressure_answer(first_char, length, raw_pressure);
        return result;
    }

#ifdef __cplusplus
}
#endif

