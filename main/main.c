#include <stdbool.h>
#include <stdint.h>

#include "button_gpio.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iot_button.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "tvbgone_core.h"

static const char *TAG = "capsule_tvbgone";

#define TVBG_HOLD_GPIO ((gpio_num_t)CONFIG_TVBG_HOLD_GPIO)
#define TVBG_BUTTON_GPIO ((gpio_num_t)CONFIG_TVBG_BUTTON_GPIO)
#define TVBG_IR_TX_GPIO ((gpio_num_t)CONFIG_TVBG_IR_TX_GPIO)
#define TVBG_RGB_LED_GPIO ((gpio_num_t)CONFIG_TVBG_RGB_LED_GPIO)

#define TVBG_SEND_TASK_STACK_SIZE 6144U
#define TVBG_SEND_TASK_PRIORITY 5U
#define TVBG_DONE_DISPLAY_MS 5000U

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} tvbg_rgb_color_t;

static const tvbg_rgb_color_t TVBG_COLOR_OFF = {0, 0, 0};
static const tvbg_rgb_color_t TVBG_COLOR_BLUE = {0, 0, 255};
static const tvbg_rgb_color_t TVBG_COLOR_RED = {255, 0, 0};
static const tvbg_rgb_color_t TVBG_COLOR_GREEN = {0, 255, 0};

static led_strip_handle_t s_led_strip;
static TaskHandle_t s_send_task;
static portMUX_TYPE s_send_lock = portMUX_INITIALIZER_UNLOCKED;

static void tvbg_set_hold(bool enabled)
{
    ESP_ERROR_CHECK(gpio_set_level(TVBG_HOLD_GPIO, enabled ? 1 : 0));
}

static void tvbg_set_led_color(tvbg_rgb_color_t color)
{
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, color.red, color.green, color.blue));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}

static void tvbg_power_off(void)
{
    ESP_LOGI(TAG, "Powering off");
    vTaskDelay(pdMS_TO_TICKS(50));
    tvbg_set_hold(false);
}

static void tvbg_sequence_task(void *arg)
{
    (void)arg;

    ESP_LOGI(TAG, "Starting EU sweep on GPIO %d", TVBG_IR_TX_GPIO);
    tvbg_set_led_color(TVBG_COLOR_BLUE);
    esp_err_t err = tvbgone_core_send(TVBGONE_CORE_REGION_EU, TVBGONE_CORE_SEND_MODE_SINGLE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "EU sweep failed: %s", esp_err_to_name(err));
        tvbg_set_led_color(TVBG_COLOR_OFF);
        goto shutdown;
    }

    ESP_LOGI(TAG, "Starting NA sweep on GPIO %d", TVBG_IR_TX_GPIO);
    tvbg_set_led_color(TVBG_COLOR_RED);
    err = tvbgone_core_send(TVBGONE_CORE_REGION_NA, TVBGONE_CORE_SEND_MODE_SINGLE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NA sweep failed: %s", esp_err_to_name(err));
        tvbg_set_led_color(TVBG_COLOR_OFF);
        goto shutdown;
    }

    ESP_LOGI(TAG, "All sweeps complete");
    tvbg_set_led_color(TVBG_COLOR_GREEN);
    vTaskDelay(pdMS_TO_TICKS(TVBG_DONE_DISPLAY_MS));

shutdown:
    taskENTER_CRITICAL(&s_send_lock);
    s_send_task = NULL;
    taskEXIT_CRITICAL(&s_send_lock);

    tvbg_power_off();
    vTaskDelete(NULL);
}

static void tvbg_start_sequence(void)
{
    bool start_task = false;

    taskENTER_CRITICAL(&s_send_lock);
    if (s_send_task == NULL) {
        start_task = true;
    }
    taskEXIT_CRITICAL(&s_send_lock);

    if (!start_task) {
        ESP_LOGI(TAG, "Ignoring trigger while send sequence is running");
        return;
    }

    BaseType_t task_created = xTaskCreate(tvbg_sequence_task, "tvbg_send",
                                          TVBG_SEND_TASK_STACK_SIZE, NULL,
                                          TVBG_SEND_TASK_PRIORITY, &s_send_task);
    if (task_created != pdPASS) {
        taskENTER_CRITICAL(&s_send_lock);
        s_send_task = NULL;
        taskEXIT_CRITICAL(&s_send_lock);
        ESP_LOGE(TAG, "Failed to create send task");
    }
}

static void tvbg_button_single_click_cb(void *arg, void *data)
{
    (void)arg;
    (void)data;
    tvbg_start_sequence();
}

static void tvbg_init_hold_gpio(void)
{
    gpio_config_t hold_cfg = {
        .pin_bit_mask = 1ULL << TVBG_HOLD_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&hold_cfg));
    tvbg_set_hold(true);
}

static void tvbg_init_led(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = TVBG_RGB_LED_GPIO,
        .max_leds = 1,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip));
    ESP_ERROR_CHECK(led_strip_clear(s_led_strip));
}

static void tvbg_init_button(void)
{
    const button_config_t button_config = {
        .short_press_time = 50,
        .long_press_time = 1000,
    };
    const button_gpio_config_t gpio_config = {
        .gpio_num = TVBG_BUTTON_GPIO,
        .active_level = 0,
        .disable_pull = false,
    };
    button_handle_t button_handle = NULL;

    ESP_ERROR_CHECK(iot_button_new_gpio_device(&button_config, &gpio_config, &button_handle));
    ESP_ERROR_CHECK(iot_button_register_cb(button_handle, BUTTON_SINGLE_CLICK, NULL,
                                           tvbg_button_single_click_cb, NULL));
}

void app_main(void)
{
    tvbgone_core_config_t tvbgone_config;

    tvbg_init_hold_gpio();
    tvbg_init_led();
    tvbg_init_button();

    tvbgone_core_get_default_config(&tvbgone_config);
    tvbgone_config.ir_led_gpio = TVBG_IR_TX_GPIO;
    tvbgone_config.rmt_channel_mode = TVBGONE_CORE_RMT_CHANNEL_MODE_INTERNAL;
    ESP_ERROR_CHECK(tvbgone_core_init(&tvbgone_config));

    ESP_LOGI(TAG, "Ready, waiting for button on GPIO %d", TVBG_BUTTON_GPIO);
    if (gpio_get_level(TVBG_BUTTON_GPIO) == 0) {
        ESP_LOGI(TAG, "Boot button is already pressed, starting sequence");
        tvbg_start_sequence();
    }

    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}
