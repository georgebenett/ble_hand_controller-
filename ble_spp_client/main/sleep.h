#ifndef SLEEP_H
#define SLEEP_H

#include <stdbool.h>
#include "esp_sleep.h"
#include "button.h"

#define SLEEP_TIMEOUT_MS 1000
#define INACTIVITY_TIMEOUT_MS 120000  // 120 seconds

void sleep_init(void);
void sleep_start_monitoring(void);
void sleep_reset_inactivity_timer(void);
void sleep_check_inactivity(bool is_ble_connected);

#endif // SLEEP_H