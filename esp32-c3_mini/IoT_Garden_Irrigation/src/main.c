#include "driver/gpio.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "esp_wifi.h"
//#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include "sntp.h"
#include "wifi.h"
#include "webserver.h"
#include "irrigation.h"
#include "crontab.h"
#include <time.h>



#ifndef WIFI_SSID
    #define WIFI_SSID "UNKNOWN_SSID"
#endif

#ifndef WIFI_PASSWORD
    #define WIFI_PASSWORD "UNKNOWN_PASSWORD"
#endif


#define TAG_LED "LED_GPIO_8"

#define STATUS_LED_GPIO GPIO_NUM_8  // LED integrado del DevKitM-1
#define SOLENOID_GPIO GPIO_NUM_0  // Salida a los solenoides
#define PUMP_GPIO GPIO_NUM_1  // Salida a la bomba

#define LED_ON 0    // El 0 enciende el LED
#define LED_OFF 1   // El 1 apaga el LED

static const char *TAG = "HTTP_SERVER_EXAMPLE";




// --- Array con todas nuestras tareas programadas ---
ScheduledTask tasks[] = {
    // Tarea 1: Todos los Lunes a las 10:34
    {
        .day_of_week = -1, // Cualquier día
        .hour = -1,
        .minute = -1,
        .second = 0,
        //.callback = tarea_del_lunes,
        //.available_this_task = true
    },
    // Tarea 2: Todos los días a las 12:00
    // (Usamos -1 para indicar "cualquier día de la semana")
    {
        .day_of_week = -1, // Cualquier día
        .hour = -1,
        .minute = -1,
        .second = 30,
        //.callback = tarea_diaria_mediodia,
        //.available_this_task = true
    }
};






void irrigation_init(irrigation_t *irrigation) {
    irrigation->status_led_pin = STATUS_LED_GPIO;
    irrigation->solenoid_pin = SOLENOID_GPIO;
    irrigation->pump_pin = PUMP_GPIO;
    irrigation->state = WAITING;
    irrigation->delayToSolenoidOn = 5;
    irrigation->delayToPumpOn = 5;
    irrigation->delayToIrrigation = 5;
    irrigation->delayToPumpOff = 5;
    irrigation->delayToSolenoidOff = 5;
    irrigation->delayToPumpRefresh = 5;
}


void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    // Configuramos el GPIO
    gpio_reset_pin(STATUS_LED_GPIO);
    gpio_set_direction(STATUS_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(STATUS_LED_GPIO, LED_OFF);

    gpio_reset_pin(SOLENOID_GPIO);
    gpio_set_direction(SOLENOID_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(SOLENOID_GPIO, 0);
    
    gpio_reset_pin(PUMP_GPIO);
    gpio_set_direction(PUMP_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(PUMP_GPIO, 0);
    
    // Retardo de inicio para darle tiempo a que funcione el monitor serial
    vTaskDelay((4 * 1000) / portTICK_PERIOD_MS);
    
    // Initialize NVS for Wi-Fi storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Starting Wi-Fi...");
    wifi_init_sta();
    // Retraso para asegurar que la conexión Wi-Fi esté establecida
    vTaskDelay((20 * 1000) / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "Starting web server...");
    start_webserver();

    ESP_LOGI(TAG, "Starting SNTP...");
    initialize_sntp();

    setenv("TZ", "CST+3", 1);
    tzset();

    ESP_LOGI(TAG, "Starting sync SNTP...");
    get_time_from_sntp();

    ESP_LOGI(TAG, "Starting irrigation periodic check...");
    irrigation_t irrigation;
    irrigation_init(&irrigation);
    initialize_irrigation_periodic_check(&irrigation);


    ESP_LOGI(TAG, "Starting crontab periodic check...");
    const uint8_t num_tasks = sizeof(tasks) / sizeof(ScheduledTask);

    crontab_tasks_t crontab_tasks = {
        .numTasks = num_tasks,
        .tasks = tasks,
        .irrigation = &irrigation
    };

    initialize_crontab_periodic_check(&crontab_tasks);
    


    
    while (1) {
        vTaskDelay((30 * 1000) / portTICK_PERIOD_MS);
    }

}
