#include "esp_check.h"
#include "esp_log.h"
#include "tvbgone_core.h"

static const char *TAG = "capsule_tvbgone";

#define CAPSULE_IR_TX_GPIO GPIO_NUM_4

void app_main(void)
{
    tvbgone_core_config_t config;

    tvbgone_core_get_default_config(&config);
    config.ir_led_gpio = CAPSULE_IR_TX_GPIO;
    config.rmt_channel_mode = TVBGONE_CORE_RMT_CHANNEL_MODE_INTERNAL;

    ESP_ERROR_CHECK(tvbgone_core_init(&config));

    ESP_LOGI(TAG, "Starting TV-B-Gone sweep on GPIO %d", CAPSULE_IR_TX_GPIO);
    ESP_ERROR_CHECK(tvbgone_core_send(TVBGONE_CORE_REGION_BOTH,
                                      TVBGONE_CORE_SEND_MODE_SINGLE));
    ESP_LOGI(TAG, "TV-B-Gone sweep finished");
}
