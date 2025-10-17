#include <time.h>
#include "esp_log.h"

static const char *TAG = "TIME_EXAMPLE";


void app_main() {
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    while (1)
    {
     time(&now);
    // Set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
    //vTaskDelay(pdMS_TO_TICKS(1000));
    esp_rom_delay_us(1 * 1000 * 1000);
    }
}