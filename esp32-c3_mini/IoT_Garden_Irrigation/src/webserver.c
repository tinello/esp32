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


/* URI handler for GET /get_scheduled */
static esp_err_t get_scheduled_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    //char *client_ip = get_and_print_client_ip(req);
    //char resp_str[128];
    //snprintf(resp_str, sizeof(resp_str), "{\"message\":\"Hello from ESP32-C3!\", \"client_ip\":\"%s\"}", client_ip);

    crontab_tasks_t *crontab_tasks = (crontab_tasks_t *)req->user_ctx;

    char resp_str[128] = "[";
    
    for (size_t i = 0; i < crontab_tasks->numTasks; i++) {
        ScheduledTask task = crontab_tasks->tasks[i];
        char task_str[64];
        snprintf(task_str, sizeof(task_str), "{\"day_of_week\":%d, \"hour\":%d, \"minute\":%d, \"second\":%d}", 
            task.day_of_week,
            task.hour,
            task.minute,
            task.second);

        if (i < crontab_tasks->numTasks - 1) {
            strncat(task_str, ",", sizeof(task_str) - strlen(task_str) - 1);
        }

        strncat(resp_str, task_str, sizeof(resp_str) - strlen(resp_str) - 1);
    }
    
    strncat(resp_str, "]", sizeof(resp_str) - strlen(resp_str) - 1);

    ESP_LOGI(TAG_SERVER, "Sending response: %s", resp_str);
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI handler for GET /get_irrigation */
static esp_err_t get_irrigation_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");

    crontab_tasks_t *crontab_tasks = (crontab_tasks_t *)req->user_ctx;

    char resp_str[256];
    snprintf(resp_str, sizeof(resp_str), "{\"irrigation_state\":%d, \"delay_to_solenoid_on\":%d, \"delay_to_pump_on\":%d, \"delay_to_irrigation\":%d, \"delay_to_solenoid_off\":%d, \"delay_to_pump_refresh\":%d}",
        crontab_tasks->irrigation->state,
        crontab_tasks->irrigation->delayToSolenoidOn,
        crontab_tasks->irrigation->delayToPumpOn,
        crontab_tasks->irrigation->delayToIrrigation,
        crontab_tasks->irrigation->delayToSolenoidOff,
        crontab_tasks->irrigation->delayToPumpRefresh);
    ESP_LOGI(TAG_SERVER, "Sending response: %s", resp_str);
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


/* URI handler for GET / */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");

    char resp_str[64];
    snprintf(resp_str, sizeof(resp_str), "{\"healthy\":true, \"version\":\"1.0.0\"}");
    ESP_LOGI(TAG_SERVER, "Sending response: %s", resp_str);
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


/* Start the web server */
httpd_handle_t start_webserver(crontab_tasks_t *crontab_tasks)
{
    httpd_uri_t root = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = root_get_handler,
        .user_ctx  = crontab_tasks
    };

    httpd_uri_t get_irrigation = {
        .uri       = "/get_irrigation",
        .method    = HTTP_GET,
        .handler   = get_irrigation_handler,
        .user_ctx  = crontab_tasks
    };

    httpd_uri_t get_scheduled = {
        .uri       = "/get_scheduled",
        .method    = HTTP_GET,
        .handler   = get_scheduled_handler,
        .user_ctx  = crontab_tasks
    };


    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG_SERVER, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        ESP_LOGI(TAG_SERVER, "Registering URI handlers");
        httpd_register_uri_handler(server, &get_scheduled);
        httpd_register_uri_handler(server, &get_irrigation);
        httpd_register_uri_handler(server, &root);
        return server;
    }

    ESP_LOGI(TAG_SERVER, "Error starting server!");
    return NULL;
}

