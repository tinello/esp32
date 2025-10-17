#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"


#define BLINK_GPIO GPIO_NUM_8  // LED integrado del DevKitM-1

static const char *TAG = "LED_BLINK_TIMER";

static void periodic_timer_callback(void* arg) {
    // Tu código de tarea periódica aquí
    ESP_LOGI(TAG, "Temporizador periódico disparado!");
    
    int level = gpio_get_level(BLINK_GPIO);
    ESP_LOGI(TAG, "Estado actual del LED: %d", level);


    if (level == 0) {
        ESP_LOGI(TAG, "Apagar LED");
        level = 1;
    } else {
        ESP_LOGI(TAG, "Encender LED");
        level = 0;
    }

    gpio_set_level(BLINK_GPIO, level);
}


void app_main() {

    // Configurar pin como salida led blink
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_INPUT_OUTPUT);


    esp_timer_handle_t periodic_timer;
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        .name = "periodic_task"
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));


    // Iniciar el temporizador para que se ejecute cada 1,000,000 microsegundos (1 segundo)
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1 * 1000000));

}