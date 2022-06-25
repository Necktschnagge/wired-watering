#pragma once

#include "secret_config.h"


#define PUMP_RELAY_MAYSON
//#define VALVE_SERVER_JAMES
//#define VALVE_SERVER_LUCAS

#ifdef PUMP_RELAY_MAYSON

static const char* string_server_name = "pump-relay-mayson";
static const char* TAG = "pump-server";

#define IP_CLIENT_CONFIG \
IP4_ADDR(&sta_ip.ip, 192, 168, 1, 10); \
IP4_ADDR(&sta_ip.gw, 192, 168, 1, 1); \
IP4_ADDR(&sta_ip.netmask, 255, 255, 255, 0);

#endif // PUMP_RELAY_MAYSON

#ifdef VALVE_SERVER_JAMES

static const char* string_server_name = "valve-server-james";
static const char* TAG = "valve-server";

#define IP_CLIENT_CONFIG \
IP4_ADDR(&sta_ip.ip, 192, 168, 1, 20); \
IP4_ADDR(&sta_ip.gw, 192, 168, 1, 1); \
IP4_ADDR(&sta_ip.netmask, 255, 255, 255, 0);

#endif // VALVE_SERVER_JAMES

#ifdef VALVE_SERVER_LUCAS

static const char* string_server_name = "valve-server-lucas";
static const char* TAG = "valve-server";

#define IP_CLIENT_CONFIG \
IP4_ADDR(&sta_ip.ip, 192, 168, 1, 21); \
IP4_ADDR(&sta_ip.gw, 192, 168, 1, 1); \
IP4_ADDR(&sta_ip.netmask, 255, 255, 255, 0);

#endif // VALVE_SERVER_LUCAS
