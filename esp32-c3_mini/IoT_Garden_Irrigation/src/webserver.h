#pragma once
#include "esp_http_server.h"
#include "irrigation.h"

httpd_handle_t start_webserver(irrigation_state_t* irrigationState);