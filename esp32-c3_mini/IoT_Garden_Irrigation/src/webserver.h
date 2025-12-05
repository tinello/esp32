#pragma once
#include "esp_http_server.h"
#include "irrigation.h"

httpd_handle_t start_webserver(crontab_tasks_t *crontab_tasks);