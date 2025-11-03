#include <time.h>
#include "esp_sntp.h"
#include "esp_log.h"
#include "esp_timer.h"


#define TAG_SNTP "SNTP"

void get_time_from_sntp(void) {
    time_t timestamp;
    struct tm timeinfo;
    
    // Espera hasta que se obtenga la hora
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG_SNTP, "Esperando por la sincronización de la hora... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    
    time(&timestamp);
    // Convierte el tiempo Unix a la estructura de fecha y hora local
    localtime_r(&timestamp, &timeinfo);

    if (timeinfo.tm_year < (2024 - 1900)) {
        ESP_LOGE(TAG_SNTP, "La hora no es válida aún.");
    } else {
        // Formatea la salida
        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG_SNTP, "Fecha y Hora actual: %s", strftime_buf);
    }
}

void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG_SNTP, "La hora ha sido sincronizada con NTP.");
}

void initialize_sntp(void) {
    ESP_LOGI(TAG_SNTP, "Inicializando SNTP");
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    
    // Configura servidores NTP. Puedes usar un servidor regional si lo prefieres.
    esp_sntp_setservername(0, "pool.ntp.org");
    
    // Asigna la función de notificación
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    
    // Inicia el proceso de sincronización
    esp_sntp_init();
}


/**************************************************************
 * Funciones de sincronización SNTP
 **************************************************************/

static void periodic_sntp_timer_callback(void* arg) {
    get_time_from_sntp();
}

void initialize_sntp_periodic_sync(void) {
    const int micro = 1000;
    const int milli = 1000;
    const int second = 60;
    const int minute = 60;
    const int hour = 2;
    const uint64_t period = (uint64_t)hour*minute*second*milli*micro; // 2 hours

    esp_timer_handle_t periodic_timer;
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_sntp_timer_callback,
        .name = "sntp_periodic_task"
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, period));
}
