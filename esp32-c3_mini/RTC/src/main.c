#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "TIME_EXAMPLE";


void app_main() {
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    // Set timezone to Argentina Standard Time
    setenv("TZ", "CST+3", 1);
    tzset();

    while (1)
    {
        time(&now);
        
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current date/time in Argentina is: %s", strftime_buf);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}