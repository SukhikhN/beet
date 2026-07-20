#include "esp_err.h"

// Initializes the MQTT client and starts the broker connection.
esp_err_t mqtt_client_init(void);

// Publishes a message.
esp_err_t mqtt_publish(const char *topic, const char *message);
