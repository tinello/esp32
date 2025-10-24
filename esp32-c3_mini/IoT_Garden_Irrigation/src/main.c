#include "driver/gpio.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "esp_sntp.h"


#ifndef WIFI_SSID
    #define WIFI_SSID "UNKNOWN_SSID"
#endif

#ifndef WIFI_PASSWORD
    #define WIFI_PASSWORD "UNKNOWN_PASSWORD"
#endif


#define TAG_LED "LED_GPIO_8"

#define BLINK_GPIO GPIO_NUM_8  // LED integrado del DevKitM-1

static const char *TAG = "HTTP_SERVER_EXAMPLE";




char *get_and_print_client_ip(httpd_req_t *req) {
    int sockfd = httpd_req_to_sockfd(req);
    struct sockaddr_in6 addr;
    socklen_t addr_size = sizeof(addr);
    
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addr_size) < 0) {
        ESP_LOGE(TAG, "Error getting client IP");
        return "";
    }

    static char ip_str[INET6_ADDRSTRLEN];
    // esp_http_server uses IPv6, so handle both IPv6 and IPv4-mapped addresses
    if (addr.sin6_family == AF_INET6) {
        inet_ntop(AF_INET6, &addr.sin6_addr, ip_str, sizeof(ip_str));
    } else {
        // Fallback for IPv4
        struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
        inet_ntop(AF_INET, &addr4->sin_addr, ip_str, sizeof(ip_str));
    }

    ESP_LOGI(TAG, "Client connected from IP: %s", ip_str);
    return ip_str;
}

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

/* URI handler for GET /hello */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    //gpio_set_level(BLINK_GPIO, 1); // apagar LED

    char *client_ip = get_and_print_client_ip(req);
    char resp_str[128];
    snprintf(resp_str, sizeof(resp_str), "<h1>Hello from ESP32-C3! %s</h1>", client_ip);
    ESP_LOGI(TAG, "Sending response: %s", resp_str);
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI handler for GET / */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    //gpio_set_level(BLINK_GPIO, 0); // encender LED

    const char* resp_str = "<h1>ESP32-C3 HTTP Server</h1><p>Visit /hello for a message.</p>";
    ESP_LOGI(TAG, "Sending response: %s", resp_str);
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    .user_ctx  = NULL
};

static httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_get_handler,
    .user_ctx  = NULL
};

/* Start the web server */
static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &root);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
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
    setenv("TZ", "GMT+3", 1); 
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


void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_VERBOSE);

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


    //initialize_sntp();


    while (1) {
        ESP_LOGI(TAG_LED, "Encendiendo LED");
        gpio_set_level(BLINK_GPIO, 0); // El 0 enciende el LED
        vTaskDelay(5000 / portTICK_PERIOD_MS);

        ESP_LOGI(TAG_LED, "Apagando LED");
        gpio_set_level(BLINK_GPIO, 1); // El 1 apaga el LED
        vTaskDelay(5000 / portTICK_PERIOD_MS);

        //get_time_from_sntp();
    }

}
