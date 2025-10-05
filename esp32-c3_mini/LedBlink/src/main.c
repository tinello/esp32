/* 
https://github.com/yanbe/ssd1306-esp-idf-i2c/blob/master/README.rst 

https://github.com/mkfrey/u8g2-hal-esp-idf/blob/master/examples/test_SSD1306_i2c.c

Revisar este repo: https://github.com/K-S-K/ESP32-02-OLed-SSD1366/tree/master
*/

#include "driver/gpio.h"
#include "esp_rom_sys.h"  // Para ets_delay_us()
#include "esp_err.h"
#include "esp_log.h"
#include "esp_task_wdt.h"



#define BLINK_GPIO GPIO_NUM_8  // LED integrado del DevKitM-1

#define TAG_LED "LED_GPIO_8"

void app_main(void)
{
	esp_task_wdt_deinit();


    // Configurar pin como salida led blink
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
	
    while (1)
    {
		//esp_task_wdt_reset();

		ESP_LOGI(TAG_LED, "Encendindo LED");
        gpio_set_level(BLINK_GPIO, 1); // Encender LED
        esp_rom_delay_us(1 * 1000 * 1000);      // 1s
		//xTaskCreate(&task_led_on, "led_on", 2048, NULL, 6, NULL);
		//vTaskDelay(pdMS_TO_TICKS(1000));      // ceder CPU al scheduler

		ESP_LOGI(TAG_LED, "Apagando LED");
        gpio_set_level(BLINK_GPIO, 0); // Apagar LED
        esp_rom_delay_us(1 * 1000 * 1000);      // 1s
		//xTaskCreate(&task_led_off, "led_off", 2048, NULL, 6, NULL);
		//vTaskDelay(pdMS_TO_TICKS(1000));      // ceder CPU al scheduler
    }
}