#pragma once

#include "esp_err.h"

// Connects the device to the configured Wi-Fi access point.
esp_err_t wifi_sta_init(void);
