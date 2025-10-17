#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BLINK_GPIO GPIO_NUM_8
#define TAG_LED "LED_GPIO_8"

void app_main(void)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        ESP_LOGI(TAG_LED, "Encendiendo LED");
        gpio_set_level(BLINK_GPIO, 0); // El 0 enciende el LED
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        ESP_LOGI(TAG_LED, "Apagando LED");
        gpio_set_level(BLINK_GPIO, 1); // El 1 apaga el LED
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}