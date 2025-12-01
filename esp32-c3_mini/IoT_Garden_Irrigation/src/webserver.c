#include "esp_http_server.h"
#include <sys/socket.h>
#include "esp_log.h"
#include "irrigation.h"


static const char *TAG_SERVER = "WEB_SERVER";


char *get_and_print_client_ip(httpd_req_t *req) {
    int sockfd = httpd_req_to_sockfd(req);
    struct sockaddr_in6 addr;
    socklen_t addr_size = sizeof(addr);
    
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addr_size) < 0) {
        ESP_LOGE(TAG_SERVER, "Error getting client IP");
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

    ESP_LOGI(TAG_SERVER, "Client connected from IP: %s", ip_str);
    return ip_str;
}


/* URI handler for GET /hello */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    /*long long manual_timestamp = 1762128000; 

    struct timeval tv;
    tv.tv_sec = (time_t)manual_timestamp;
    tv.tv_usec = 0;

    if (settimeofday(&tv, NULL) != 0) {
        printf("Error al establecer la hora.\n");
    } else {
        printf("Hora establecida manualmente.\n");
    }*/

    httpd_resp_set_type(req, "application/json");
    char *client_ip = get_and_print_client_ip(req);
    char resp_str[128];
    snprintf(resp_str, sizeof(resp_str), "{\"message\":\"Hello from ESP32-C3!\", \"client_ip\":\"%s\"}", client_ip);
    ESP_LOGI(TAG_SERVER, "Sending response: %s", resp_str);
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI handler for GET / */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");

    irrigation_state_t *irrigationState = (irrigation_state_t *)req->user_ctx;

    char resp_str[128];
    snprintf(resp_str, sizeof(resp_str), "{\"healthy\":true, \"irrigation_state\":%d}", *irrigationState);
    ESP_LOGI(TAG_SERVER, "Sending response: %s", resp_str);
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


/* Start the web server */
httpd_handle_t start_webserver(irrigation_state_t* irrigationState)
{
    httpd_uri_t hello = {
        .uri       = "/hello",
        .method    = HTTP_GET,
        .handler   = hello_get_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t root = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = root_get_handler,
        .user_ctx  = irrigationState
    };

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG_SERVER, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        ESP_LOGI(TAG_SERVER, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &root);
        return server;
    }

    ESP_LOGI(TAG_SERVER, "Error starting server!");
    return NULL;
}

