#include "driver/gpio.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sntp.h"
#include "wifi.h"
#include "webserver.h"



#ifndef WIFI_SSID
    #define WIFI_SSID "UNKNOWN_SSID"
#endif

#ifndef WIFI_PASSWORD
    #define WIFI_PASSWORD "UNKNOWN_PASSWORD"
#endif


#define TAG_LED "LED_GPIO_8"

#define BLINK_GPIO GPIO_NUM_8  // LED integrado del DevKitM-1

static const char *TAG = "HTTP_SERVER_EXAMPLE";






void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    vTaskDelay((4 * 1000) / portTICK_PERIOD_MS);

    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    // Initialize NVS for Wi-Fi storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Starting Wi-Fi...");
    wifi_init_sta();

    ESP_LOGI(TAG, "Starting web server...");
    start_webserver();


    initialize_sntp();


    while (1) {
        ESP_LOGI(TAG_LED, "Encendiendo LED");
        gpio_set_level(BLINK_GPIO, 0); // El 0 enciende el LED
        vTaskDelay((30 * 1000) / portTICK_PERIOD_MS);

        ESP_LOGI(TAG_LED, "Apagando LED");
        gpio_set_level(BLINK_GPIO, 1); // El 1 apaga el LED
        vTaskDelay((30 * 1000) / portTICK_PERIOD_MS);

        get_time_from_sntp();
    }

}
