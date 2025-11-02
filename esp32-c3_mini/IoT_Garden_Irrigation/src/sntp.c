#include "esp_sntp.h"
#include "esp_log.h"


#define TAG_SNTP "SNTP"

void get_time_from_sntp(void) {
    time_t now;
    struct tm timeinfo;
    
    // Espera hasta que se obtenga la hora
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG_SNTP, "Esperando por la sincronización de la hora... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    
    time(&now);
    
    // Ajusta la zona horaria (ejemplo para Argentina: GMT-3)
    // Para otras zonas, busca el string de TZ (ej: "CET-1CEST,M3.5.0,M10.5.0/3")
    setenv("TZ", "GMT+3", 1); 
    tzset();
    
    // Convierte el tiempo Unix a la estructura de fecha y hora local
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year < (2016 - 1900)) {
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

