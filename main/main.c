#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_websocket_client.h"

#include "dht.h"

static const char *TAG = "Humidity sensor";

#define WIFI_SSID      "Veliyamkol"
#define WIFI_PASS      "Vel@pera6"
#define WIFI_MAX_RETRY 5

static int s_retry_num = 0;
static char s_ip_str[16];

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying to connect to the AP");
        } else {
            ESP_LOGI(TAG, "Connect to the AP fail");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        snprintf(s_ip_str, sizeof(s_ip_str), "%d.%d.%d.%d", IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }
}

static void wifi_init_sta(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_got_ip);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

static esp_websocket_client_handle_t websocket_client = NULL;

static void websocket_app_start(void) {    
    esp_websocket_client_config_t websocket_cfg = {
        .uri = "ws://weatherapi.glitchy.systems",
    };
    websocket_client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_client_start(websocket_client);
    ESP_LOGI(TAG, "WebSocket client started");
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    wifi_init_sta();

    websocket_app_start();

    // Select sensor type and GPIO pin from menuconfig
    dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
    gpio_num_t gpio_num = 33;

    // Enable internal pull-up resistor if specified in menuconfig
    gpio_pullup_en(gpio_num);

    char ws_msg[64];
    while (1)
    {
        float humidity = 0, temperature = 0;

        // Read data from the sensor
        esp_err_t result = dht_read_float_data(sensor_type, gpio_num, &humidity, &temperature);
        if (result == ESP_OK)
        {
            ESP_LOGI(TAG, "Humidity: %.1f%% Temperature: %.1f°C", humidity, temperature);
            if (websocket_client && esp_websocket_client_is_connected(websocket_client)) {
                snprintf(ws_msg, sizeof(ws_msg), "{\"humidity\":%.1f,\"temperature\":%.1f}", humidity, temperature);
                esp_websocket_client_send_text(websocket_client, ws_msg, strlen(ws_msg), portMAX_DELAY);
            }
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read sensor data: %s", esp_err_to_name(result));
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}