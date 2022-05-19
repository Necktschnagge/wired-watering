#pragma once

#include "secret_config.h"
#define WIFI_CONNECT_MAX_RETRY 250

#define IP_CLIENT_CONFIG IP4_ADDR(&sta_ip.ip, 192, 168, 1, 10); \
IP4_ADDR(&sta_ip.gw, 192, 168, 1, 1); \
IP4_ADDR(&sta_ip.netmask, 255, 255, 255, 0);

#define PUMP_RELAY_MAYSON
//#define VALVE_SERVER_JAMES

#ifdef PUMP_RELAY_MAYSON
static const char* string_server_name = "pump-relay-mayson";
static const char* TAG = "pump-server";
#endif // PUMP_RELAY_MAYSON

#ifdef VALVE_SERVER_JAMES
static const char* string_server_name = "valve-server-james";
static const char* TAG = "valve-server";
#endif // PUMP_RELAY_MAYSON
