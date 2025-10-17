#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_https_server.h" // Se incluye la biblioteca HTTPS

// Certificados SSL generados
//#include "cert.h"
//#include "privkey.h"

#ifndef WIFI_SSID
    #define WIFI_SSID "UNKNOWN_SSID"
#endif

#ifndef WIFI_PASSWORD
    #define WIFI_PASSWORD "UNKNOWN_PASSWORD"
#endif


static const char *TAG = "HTTPS_SERVER_EXAMPLE";

// Incrustar el certificado PEM directamente en el código
const char servercert_pem[] = "MIIDEzCCAfugAwIBAgIUBrbcjuvY/1aKcz29cCScKxfVSTQwDQYJKoZIhvcNAQELBQAwGTEXMBUGA1UEAwwOZXNwMzItYzMtaHR0cHMwHhcNMjUxMDA1MjMyMTI1WhcNMjYxMDA1MjMyMTI1WjAZMRcwFQYDVQQDDA5lc3AzMi1jMy1odHRwczCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKyYZ6Y5HVvtco+93Y7ddjf0u7eGMsNkkf70xIHtxtzzKHn2dIbgXie4utX23W/K/3+A3KOkK1yR8B5LSmrhJndjBXAqpS1gGAl/5hqxbBEkFG0CpVIzVMmoe58ECEBwxW6JGi8EmnFhCPilY37xkA+eirvXCtHhC4/85EAxGyJtWMYQ62wchNaPZdMlwdr4ssGPJMZm5hxpQ9jHIq8pwJnEbjTdOZStzOFotuHQTekfKHwMxowO0PahPXsD1aYeVKI80Wi/CZl+A54IizCf4mOVNjkCniY6lZaY2RMyqm+ddJE6xHZwF9e/a5l0FP6b4n+88oQRKOsHiu+d/3/+/Z0CAwEAAaNTMFEwHQYDVR0OBBYEFFRaJ4mGqaKVe5HiBcNy+c7u33DIMB8GA1UdIwQYMBaAFFRaJ4mGqaKVe5HiBcNy+c7u33DIMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAH2pYM8H2nCc0vGAWYZxbLOdcZ0JdLOmqHCYsKc6RTSQoPzK3W1ODW+fRRZriI08nv7kHYepcR3zJIRekiX6EDA9S8CL2R5ajTv7uCljYTZdY8fmX41cdoB8rBuz2eRtNAiuaKgUoD7YIZF+C58n9eHleEweY4Bs4jrz2gXDLMDNa1oahX1b/GuH57HO+nNGuM/DgugbOWoLPlt/GVxkrHR5+kWs/AlnaYAaCBMHR0/T51prem9aywaoXGTiI8OlVp1Bid90FXwS3fN7Y3RbtuUMF+1RaPpXv9Y01xgZ6vlBo+gJKlEaUdPaJFQTBbHD0dHCJ1jCBY530h5U2Bj92PU=";

// Incrustar la clave privada PEM directamente en el código
const char prvtkey_pem[] = "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCsmGemOR1b7XKPvd2O3XY39Lu3hjLDZJH+9MSB7cbc8yh59nSG4F4nuLrV9t1vyv9/gNyjpCtckfAeS0pq4SZ3YwVwKqUtYBgJf+YasWwRJBRtAqVSM1TJqHufBAhAcMVuiRovBJpxYQj4pWN+8ZAPnoq71wrR4QuP/ORAMRsibVjGEOtsHITWj2XTJcHa+LLBjyTGZuYcaUPYxyKvKcCZxG403TmUrczhaLbh0E3pHyh8DMaMDtD2oT17A9WmHlSiPNFovwmZfgOeCIswn+JjlTY5Ap4mOpWWmNkTMqpvnXSROsR2cBfXv2uZdBT+m+J/vPKEESjrB4rvnf9//v2dAgMBAAECggEADB3zAOd3mTGVXmQM/u+rUghbeWnxfGg+G7EhbqG72V001UMpMSLlWVKNw2G+cA/HWjM4ykBHTwtr2SSeB9zgr3cDNbm4uPPLHDefdvZHyPhIetK5uoiiBjpO234T+jowki8L6IshMJ6IW7R4xLu+ymnuAXv6ZXuwLR6rA5qu9f6rq8OoJeJdp212p3NCVUdL2o8NrSFXyYCmrSKmrh2JYrROms2NT8t0QaI3or1CcIQU4+zHpBT/vGiqp2UiChgn5r32rbl7FYCNkByDJfHyPgvUWabcv9RVH4ls+h62Zaia/wFVoZ0oqy7dLZQubMbaLh/co8BtpO6G+WpBXTK6oQKBgQDhXFedo5vVxZvdf8Fsqyuw4VFIRTdfzoSpWZMjPBG+/2N+I9+atA0MvWTvOgHb5HpooVKUr3C9X0OH+fTKzarkBAcOKzeF4h2k7O1xF3lz8wfiF5UlYRS6FCeT6SRmK7V5kM3Ecrf3RFtpf52xRpj8Hkp7U3zZrw96qrRfkSvf9QKBgQDED5GU6rFWxuNH9HmLLu5iTuo4ombV4NBlbZpGStqbfXaCSEnJFsdcpr4agYgpedkjyJtk8nLMdedqKf5v/IzsXgJFrZRgxnmgeiEYvMhHKH6Bngyw+9Zn885wT4TOkNsz1PPWEJVyoDbAsv6d+e3tDwq1wC6+jM3zAXbLwIbmCQKBgQDakb8ASw/3E0MKXr/wb0tesAkveMIuD5Qm05ObFsV3YybRzmUuG/aUJpRgWPg4lq4KEHyRbF6BrIVIuiqrDzHxF97n5Q5isV1i+c+IYeYPiORaadjjefaqSqXgFLySw02s675GC6VVEl0+QuFkcD4sOAyw89YhImJ27F0oWxociQKBgEWyBx155HqljuvoQ0Cv+TvnM6nYSVgkZ9B2is4ME/QMQJGFov0h5NLRz2havY2I7CClUmArM5XtH8uEjBdBHtNzg7lDQCi6xUe/wn4ptcRd58Kp7XcIvyoK6ErSabKImfOt8tQY5xdtTqlnt81uqB6KJ1N+st837IpC7fj2Qr4JAoGAFGRvZmX5fxoJBVXfazCoCZSlAWji6alwt9cXTG43eO72i+QuBd4kRn//u1Ooj2GQcjrQ3+uqSfLycvzNCbU/NmH33p+S197rmAeQs6gqzFWwjK2L1drcoQymUk9yoI3wW2gE+DCw+9ypWsnYoscng4GHscndifFsT1pINUGdBRA=";

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
    esp_wifi_start();
}

/* URI handler for GET /hello */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    const char* resp_str = "<h1>Hello desde ESP32-C3 via HTTPS!</h1>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI handler for GET / */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    const char* resp_str = "<h1>Servidor HTTPS ESP32-C3</h1><p>Visita /hello para un mensaje seguro.</p>";
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

/* Start the HTTPS web server */
static httpd_handle_t start_webserver(void)
{
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();

    config.cacert_pem = (const uint8_t*)servercert_pem;
    config.cacert_len = strlen(servercert_pem);
    config.prvtkey_pem = (const uint8_t*)prvtkey_pem;
    config.prvtkey_len = strlen(prvtkey_pem);

    httpd_handle_t server = NULL;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.port_secure);
    if (httpd_ssl_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &root);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void app_main(void)
{
    ESP_LOGI("PEPE", "nvs_flash_init");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI("PEPE", "nvs_flash_erase");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_LOGI("PEPE", "nvs_flash_init 2");
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("PEPE", "wifi_init_sta");
    wifi_init_sta();

    ESP_LOGI("PEPE", "Starting web server");
    start_webserver();
}
