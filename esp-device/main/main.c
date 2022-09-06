
#include "atm_slave_comm.h"
#include "config.h"

#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <esp_http_server.h>
#include "driver/gpio.h"
#include <string.h>


#include "gpio_definitions_felix.h"
#include "gpio_definitions_lucas.h"
#include "gpio_definitions_james.h"
#include "gpio_definitions_mayson.h"
#include "gpio_wrapper.h"
#include "server_c_connector.h"

#include "custom_utils.h"

static const char* string_on = "on";
static const char* string_off = "off";

#ifdef PUMP_RELAY_MAYSON
static bool pump_system = true;
static bool pump_manual = false;
static bool pump_auto = false;
#endif // PUMP_RELAY_MAYSON

#ifdef ANY_VALVE_SERVER
static unsigned long global_valve_state = 0;
static uint16_t global_pressure_value = 0;
#endif // ANY_VALVE_SERVER


static bool wifi_connected = false;

/* pressure get handler */
esp_err_t pressure_get_handler(httpd_req_t* req)
{
    char* buf;
    size_t buf_len;
    bool abort_on_wrong_target_name = false;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(logging_tag, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "target-name") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "target-name", buf, buf_len) == ESP_OK) {
            ESP_LOGI(logging_tag, "Found header => target-name: %s", buf);
            if (strcmp(buf, string_server_name) != 0) {
                ESP_LOGI(logging_tag, "Header target-name does not match device name \"%s\". Ignoring request.", string_server_name);
                abort_on_wrong_target_name = true;
            }
        }
        free(buf);
    }

    if (abort_on_wrong_target_name) {
        /* Set some custom headers */
        //httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
        //httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

        /* Send response with custom headers and body set as the
         * string passed in user context*/
        const char* resp_str = "{\n \"error\":\"wrong target name\"\n}";
        httpd_resp_send(req, resp_str, strlen(resp_str));

        /* After sending the HTTP response the old HTTP request
         * headers are lost. Check if HTTP request headers can be read now. */
         /*if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
             ESP_LOGI(logging_tag, "Request headers lost");
         }
         */
        return ESP_OK;
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(logging_tag, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */

#if false
            if (httpd_query_key_value(buf, "valves", param, sizeof(param)) == ESP_OK) {
                const char* end;
                unsigned long valve_value = strtoul(param, &end, 10);
                global_valve_state = valve_value;
                ESP_LOGI(logging_tag, "got valves =%s", param);
            }
#endif // inactive
            free(buf);
        }
    }

    ESP_LOGI(logging_tag, "Starting to request sensor value....");




    /* Set some custom headers */
    httpd_resp_set_hdr(req, "server-name", string_server_name);
    //httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
     //const char* resp_str = "{\n \"server-name\" : \"pump-relay-mayson\"\n \"manual\" : XXXXX\n \"auto\" : XXXXX\n \"system\" : XXXXX\n}";
    char resp_str[200] = "";
#ifdef ANY_VALVE_SERVER
    int okn = c_for_get_set_valve_answer(resp_str, 200);
#endif // ANY_VALVE_SERVER
#ifdef PUMP_RELAY_MAYSON
    int okn = c_for_get_relay_answer(resp_str, 200);
#endif // PUMP_RELAY_MAYSON


    //(const char*) custom_answer;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
     /*if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
         ESP_LOGI(logging_tag, "Request headers lost");
     }*/
    return ESP_OK;
}


/* An HTTP GET handler */
esp_err_t status_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;
    bool abort_on_wrong_target_name = false;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(logging_tag, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "target-name") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "target-name", buf, buf_len) == ESP_OK) {
            ESP_LOGI(logging_tag, "Found header => target-name: %s", buf);
            if (strcmp(buf, string_server_name) != 0) {
                ESP_LOGI(logging_tag, "Header target-name does not match device name \"%s\". Ignoring request.", string_server_name);
                abort_on_wrong_target_name = true;
            }
        }
        free(buf);
    }

    if (abort_on_wrong_target_name) {
        /* Set some custom headers */
        //httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
        //httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

        /* Send response with custom headers and body set as the
         * string passed in user context*/
        const char* resp_str = "{\n \"error\":\"wrong target name\"\n}";
        httpd_resp_send(req, resp_str, strlen(resp_str));

        /* After sending the HTTP response the old HTTP request
         * headers are lost. Check if HTTP request headers can be read now. */
        /*if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
            ESP_LOGI(logging_tag, "Request headers lost");
        }
        */
        return ESP_OK;
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(logging_tag, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
#ifdef PUMP_RELAY_MAYSON
            if (httpd_query_key_value(buf, "manual", param, sizeof(param)) == ESP_OK) {
                if (strcmp(param, string_on) == 0) {
                    pump_manual = true;
                }
                if (strcmp(param, string_off) == 0) {
                    pump_manual = false;
                }
                ESP_LOGI(logging_tag, "Keep / set manual =%s", pump_manual ? string_on : string_off);
            }
            if (httpd_query_key_value(buf, "auto", param, sizeof(param)) == ESP_OK) {
                if (strcmp(param, string_on) == 0) {
                    pump_auto = true;
                }
                if (strcmp(param, string_off) == 0) {
                    pump_auto = false;
                }
                ESP_LOGI(logging_tag, "Keep / set auto =%s", pump_auto ? string_on : string_off);
            }
            if (httpd_query_key_value(buf, "system", param, sizeof(param)) == ESP_OK) {
                if (strcmp(param, string_on) == 0) {
                    pump_system = true;
                }
                if (strcmp(param, string_off) == 0) {
                    pump_system = false;
                }
                ESP_LOGI(logging_tag, "Keep / set system =%s", pump_system ? string_on : string_off);
            }
#endif // PUMP_RELAY_MAYSON
#ifdef ANY_VALVE_SERVER
            if (httpd_query_key_value(buf, "valves", param, sizeof(param)) == ESP_OK) {
                const char* end;
                unsigned long valve_value = strtoul(param, &end, 10);
                global_valve_state = valve_value;
                ESP_LOGI(logging_tag, "got valves =%s", param);
            }
#endif // ANY_VALVE_SERVER



            free(buf);
        }
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "server-name", string_server_name);
    //httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    //const char* resp_str = "{\n \"server-name\" : \"pump-relay-mayson\"\n \"manual\" : XXXXX\n \"auto\" : XXXXX\n \"system\" : XXXXX\n}";
    char resp_str[200] = "";
#ifdef ANY_VALVE_SERVER
    int okn = c_for_get_set_valve_answer(resp_str, 200);
#endif // ANY_VALVE_SERVER
#ifdef PUMP_RELAY_MAYSON
    int okn = c_for_get_relay_answer(resp_str, 200);
#endif // PUMP_RELAY_MAYSON


    //(const char*) custom_answer;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    /*if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(logging_tag, "Request headers lost");
    }*/
    return ESP_OK;
}


httpd_uri_t status_uri = {
    .uri       = "/status",
    .method    = HTTP_GET,
    .handler   = status_get_handler,
    // Let's pass response string in user context to demonstrate it's usage 
    //.user_ctx  = "Hello World!"
};

httpd_uri_t pressure_uri = {
    .uri = "/pressure",
    .method = HTTP_GET,
    .handler = pressure_get_handler,
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(logging_tag, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(logging_tag, "Registering URI handlers");
        httpd_register_uri_handler(server, &status_uri);
#ifdef VALVE_SERVER_JAMES
        httpd_register_uri_handler(server, &pressure_uri);
#endif // VALVE_SERVER_JAMES
        return server;
    }

    ESP_LOGE(logging_tag, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

typedef struct global_entities_s {
    httpd_handle_t httpd_server_handle;
} global_entities_t;

global_entities_t GLOBAL;

void init_GLOBAL() {
    GLOBAL.httpd_server_handle = NULL;
}

static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(logging_tag, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(logging_tag, "Starting webserver");
        *server = start_webserver();
    }
}

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group; //check if it is needed anymore #### ? TODO

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int retry_delay_s = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = false;
        ESP_LOGI(logging_tag, "Wait %i seconds before trying to reconnect Wifi.", retry_delay_s);
        vTaskDelay(retry_delay_s * 1000 / portTICK_RATE_MS);
            esp_wifi_connect();
            retry_delay_s = (retry_delay_s * 49 + 60 * 2) / 50; // stabilizes on 71
            ESP_LOGI(logging_tag, "retry to connect to the AP");
        ESP_LOGI(logging_tag, "connect to the AP fail");
        //##### handle the case where it does not reconnect anymore !!!
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        wifi_connected = true;
        ESP_LOGI(logging_tag, "got ip:%s",
            ip4addr_ntoa(&event->ip_info.ip));
        retry_delay_s = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void connect_wifi(void) {
    
    s_wifi_event_group = xEventGroupCreate(); // event group for signals about wifi being connected or not

    ESP_LOGI(logging_tag, "initializing adapter...");

    tcpip_adapter_init();

    tcpip_adapter_ip_info_t sta_ip;
    uint8_t sta_mac[6];

    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac)); //<<<<< needed?
    ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &sta_ip)); //<<<<< needed?

    ESP_LOGI(logging_tag, "setting static ip adresses...");

    /* set static ip, gateway and netmask */
    IP_CLIENT_CONFIG;

    ESP_ERROR_CHECK(tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA)); // stop DHCP client in order to use static ip addresses.
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &sta_ip));

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_LOGI(logging_tag, "registering wifi handlers...");

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD
        },
    };

    /* Setting a password implies station will connect to all security modes including WEP/WPA.
        * However these modes are deprecated and not advisable to be used. Incase your Access point
        * doesn't support WPA2, these mode can be enabled by commenting below line */

    if (strlen((char*)wifi_config.sta.password)) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(logging_tag, "issued wifi start");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        wifi_connected = true;
        ESP_LOGI(logging_tag, "connected to ap SSID:%s",
            WIFI_SSID);
    }
    else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(logging_tag, "Failed to connect to SSID:%s",
            WIFI_SSID);
    }
    else {
        ESP_LOGE(logging_tag, "UNEXPECTED EVENT");
    }

    /* finish wifi
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
    */
}

void gpio_actor(void) {

#ifdef PUMP_RELAY_MAYSON

    unsigned int cnt = 0;
    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS);
        int x = gpio_get_level(PIN_D3);
        int y = gpio_get_level(PIN_D4);
        if (!x) {
            pump_system = !pump_system;
        }
        if (!y) {
            pump_manual = !pump_manual;
        }
        gpio_set_level(PIN_D1, pump_system);
        gpio_set_level(PIN_D2, wifi_connected); // wifi
        gpio_set_level(PIN_D0, pump_auto);
        gpio_set_level(PIN_D6, pump_manual);
        gpio_set_level(PIN_D7, pump_system && (pump_auto || pump_manual)); // relay
        if (!x || !y) {
            vTaskDelay(2000 / portTICK_RATE_MS);
        }
        ++cnt;
    }
#endif // PUMP_RELAY_MAYSON
#ifdef VALVE_SERVER_JAMES
    unsigned int cnt = 0;

    int got_data = gpio_get_level(JAMES_ATM_TO_ESP_DATA);
    int got_clock = gpio_get_level(JAMES_ATM_TO_ESP_CLOCK);

again_sync:
    wait_to_not_be_busy;
    TickType_t t0 = xTaskGetTickCount(); //uint32_t

    // start sync
    gpio_set_level(JAMES_ESP_TO_ATM_SYNC, 0);
    gpio_set_level(JAMES_ESP_TO_ATM_CLOCK, 0);
    gpio_set_level(JAMES_ESP_TO_ATM_DATA, 0);

    wait_to_not_be_busy;
    gpio_set_level(JAMES_ESP_TO_ATM_DATA, 0); // data LOW
    gpio_set_level(JAMES_ESP_TO_ATM_CLOCK, 0); // clock LOW
    gpio_set_level(JAMES_ESP_TO_ATM_SYNC, 1);
    wait_to_not_be_busy;
    while (gpio_get_level(JAMES_ATM_TO_ESP_DATA) == 0 || gpio_get_level(JAMES_ATM_TO_ESP_CLOCK) == 0) {
        //wait until data in, clock in is HIGH
        if (xTaskGetTickCount() - t0 > 100) {
            ESP_LOGI(logging_tag, "timeout #4301132");
            goto again_sync;
        }
        wait_to_not_be_busy;
    }
    /*while (xTaskGetTickCount() - t < 100) //while 10ms not over ####might be the wrong dividend,
    {
        wait_to_not_be_busy;
        if (gpio_get_level(ATM_TO_ESP_DATA) == 0 || gpio_get_level(ATM_TO_ESP_CLOCK) == 0) {
            goto again_sync;
        }
    }
    */
    gpio_set_level(JAMES_ESP_TO_ATM_CLOCK, 1); // clock HIGH
    while (gpio_get_level(JAMES_ATM_TO_ESP_CLOCK) == 1) {
        //wait until clock in is LOW
        if (xTaskGetTickCount() - t0 > 100) {
            ESP_LOGI(logging_tag, "timeout #4471132");
            goto again_sync;
        }
        wait_to_not_be_busy;

    }
    gpio_set_level(JAMES_ESP_TO_ATM_SYNC, 0); // sync LOW
    gpio_set_level(JAMES_ESP_TO_ATM_CLOCK, 0); // clock LOW
    while (gpio_get_level(JAMES_ATM_TO_ESP_CLOCK) == 0) {
        //wait until clock in is LOW
        if (xTaskGetTickCount() - t0 > 100) {
            ESP_LOGI(logging_tag, "timeout #4581132");
            goto again_sync;
        }
        wait_to_not_be_busy;
    }

    //sync ready

    while (1) {

        uint8_t OPCODE_SET_VALVES = 1; //4 bit OPCODES

        bool success_1 = send_bits_u8(OPCODE_SET_VALVES, 4, t0, 100); // always send LSB first
        if (!success_1) goto again_sync;

        long valve_output = global_valve_state; // make it interrupt-safe!!!

        bool success_2 = send_bits_u8(valve_output, 8, t0, 100);
        if (!success_2) goto again_sync;

        ESP_LOGI(logging_tag, "finished sending");

        uint8_t OPCODE_GET_PRESSURE = 2; //4 bit OPCODES

        bool success_3 = send_bits_u8(OPCODE_GET_PRESSURE, 4, t0, 100); // always send LSB first
        if (!success_3) goto again_sync;

        uint16_t pressure_value;

        bool success_4 = read_bits_u16(&pressure_value, 16, t0, 100);
        if (!success_4) goto again_sync;

        global_pressure_value = pressure_value;

        ESP_LOGI(logging_tag, "finished receiving pressure value");
        ++cnt;
    }

#endif // VALVE_SERVER_JAMES

#ifdef VALVE_SERVER_LUCAS

    while (1) {
        vTaskDelay(1); // do not block for concurrent procedures
        long valve_output = global_valve_state; // make it interrupt-safe!!!
        gpio_set_level(LUCAS_VALVE_0, (valve_output >> 0) % 2);
        gpio_set_level(LUCAS_VALVE_1, (valve_output >> 1) % 2);
        gpio_set_level(LUCAS_VALVE_2, (valve_output >> 2) % 2);
    }

#endif // VALVE_SERVER_LUCAS

#ifdef VALVE_SERVER_FELIX

    while (1) {
        vTaskDelay(1); // do not block for concurrent procedures
        long valve_output = global_valve_state; // make it interrupt-safe!!!
        gpio_set_level(FELIX_VALVE_0, (valve_output >> 0) % 2);
        gpio_set_level(FELIX_VALVE_1, (valve_output >> 1) % 2);
    }

#endif // VALVE_SERVER_FELIX
}

void app_main()
{
    ESP_LOGI(logging_tag, "Initializing GLOBAL...");
    init_GLOBAL();

    ESP_LOGI(logging_tag, "Initializing NVS partition...");
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_LOGI(logging_tag, "Initializing GPIO...");
    gpio_init();

    ESP_LOGI(logging_tag, "Connecting to WLAN Access Point...");
    connect_wifi();

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &GLOBAL.httpd_server_handle)); // handler to start http server on WLAN connected event

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &GLOBAL.httpd_server_handle)); // handler to stop http server on WLAN disconnected event

    ESP_LOGI(logging_tag, "Starting http server...");
    GLOBAL.httpd_server_handle = start_webserver();

    ESP_LOGI(logging_tag, "Setting / getting gpio permanently...");
    gpio_actor(); // should never return

    ESP_LOGE(logging_tag, "This section should never be reached!");
    // should never be reached:
    while (true) {
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}
