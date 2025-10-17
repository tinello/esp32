#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_timer.h"

#include "esp_event.h"


#ifndef WIFI_SSID
    #define WIFI_SSID "UNKNOWN_SSID"
#endif

#ifndef WIFI_PASSWORD
    #define WIFI_PASSWORD "UNKNOWN_PASSWORD"
#endif


static const char *TAG = "PRUEBA_SNTP";

/* Event handler for Wi-Fi events */
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "retrying to connect to the AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

/* Wi-Fi initialization */
static void wifi_init_sta(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &event_handler,
                                        NULL,
                                        &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &event_handler,
                                        NULL,
                                        &instance_got_ip);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_netif_set_hostname(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), "ESP32-C3 mini oled");
    esp_wifi_start();
}

void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "La hora ha sido sincronizada con NTP.");
}

void initialize_sntp(void) {
    ESP_LOGI(TAG, "Inicializando SNTP");
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    
    // Configura servidores NTP. Puedes usar un servidor regional si lo prefieres.
    esp_sntp_setservername(0, "pool.ntp.org");
    
    // Asigna la función de notificación
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    
    // Inicia el proceso de sincronización
    esp_sntp_init();
}

void get_time_from_sntp(void) {
    time_t now;
    struct tm timeinfo;
    
    // Espera hasta que se obtenga la hora
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Esperando por la sincronización de la hora... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    
    time(&now);
    
    // Ajusta la zona horaria (ejemplo para Argentina: GMT-3)
    // Para otras zonas, busca el string de TZ (ej: "CET-1CEST,M3.5.0,M10.5.0/3")
    setenv("TZ", "GMT+3", 1); // Nose porque, tengo que usar +3, algun dia lo arreglare.
    tzset();
    
    // Convierte el tiempo Unix a la estructura de fecha y hora local
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGE(TAG, "La hora no es válida aún.");
    } else {
        // Formatea la salida
        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "Fecha y Hora actual: %s", strftime_buf);
    }
}

static void periodic_timer_callback(void* arg) {
    // Tu código de tarea periódica aquí
    ESP_LOGI(TAG, "Temporizador periódico disparado!");
    get_time_from_sntp();
}

void app_main() {

    esp_log_level_set("*", ESP_LOG_VERBOSE);


    ESP_LOGI(TAG, "Starting delay...");
    esp_rom_delay_us(3 * 1000 * 1000);
    ESP_LOGI(TAG, "Ending delay...");
    ESP_LOGI(TAG, "SSID: %s", WIFI_SSID);
    ESP_LOGI(TAG, "Password: %s", WIFI_PASSWORD);

    // Initialize NVS for Wi-Fi storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if (CONFIG_LOG_MAXIMUM_LEVEL > CONFIG_LOG_DEFAULT_LEVEL) {
        /* If you only want to open more logs in the wifi module, you need to make the max level greater than the default level,
         * and call esp_log_level_set() before esp_wifi_init() to improve the log level of the wifi module. */
        esp_log_level_set("wifi", CONFIG_LOG_MAXIMUM_LEVEL);
    }

    ESP_LOGI(TAG, "Starting Wi-Fi...");
    wifi_init_sta();


    initialize_sntp();


    esp_timer_handle_t periodic_timer;
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        .name = "periodic_task"
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));


    // Iniciar el temporizador para que se ejecute cada 60,000,000 microsegundos (60 segundos)
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 60 * 1000000));
}