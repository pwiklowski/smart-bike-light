#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include <led_strip.h>
#include "esp_log.h"
#include "light.h"
#include "nvs_flash.h"

AppData app_data;

#define LIGHT_TAG "LIGHT"
#define LIGHT_SETTINGS_KEY "SETTINGS"

#define CHAR_UUID_FRONT_LIGHT_TOGGLE        0xFF01
#define CHAR_UUID_FRONT_LIGHT_MODE          0xFF02
#define CHAR_UUID_FRONT_LIGHT_SETTING       0xFF03

#define CHAR_UUID_BACK_LIGHT_TOGGLE        0xFF04
#define CHAR_UUID_BACK_LIGHT_MODE          0xFF05
#define CHAR_UUID_BACK_LIGHT_SETTING       0xFF06

#define LED_STRIP_LENGTH 5
#define LED_STRIP_RMT_INTR_NUM 19U

static struct led_color_t led_strip_buf_1[LED_STRIP_LENGTH];
static struct led_color_t led_strip_buf_2[LED_STRIP_LENGTH];
static struct led_strip_t led_strip = {
      .rgb_led_type = RGB_LED_TYPE_WS2812,
      .rmt_channel = RMT_CHANNEL_1,
      .rmt_interrupt_num = LED_STRIP_RMT_INTR_NUM,
      .gpio = GPIO_NUM_22,
      .led_strip_buf_1 = led_strip_buf_1,
      .led_strip_buf_2 = led_strip_buf_2,
      .led_strip_length = LED_STRIP_LENGTH
};


static struct led_color_t led_strip_back_buf_1[LED_STRIP_LENGTH];
static struct led_color_t led_strip_back_buf_2[LED_STRIP_LENGTH];
static struct led_strip_t led_strip_back = {
      .rgb_led_type = RGB_LED_TYPE_WS2812,
      .rmt_channel = RMT_CHANNEL_2,
      .rmt_interrupt_num = LED_STRIP_RMT_INTR_NUM,
      .gpio = GPIO_NUM_19,
      .led_strip_buf_1 = led_strip_back_buf_1,
      .led_strip_buf_2 = led_strip_back_buf_2,
      .led_strip_length = LED_STRIP_LENGTH
};

void light_settings_load() {
  esp_err_t err;
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGE(LIGHT_TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
    size_t len = 100;
    err = nvs_get_blob(my_handle, LIGHT_SETTINGS_KEY, &app_data, &len);
    if (err != ESP_OK) {
      ESP_LOGE(LIGHT_TAG, "Failed to load settings %d", err);
    }
    nvs_close(my_handle);
  }
}

void light_settings_save() {
  esp_err_t err;
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGE(LIGHT_TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
    err = nvs_set_blob(my_handle, LIGHT_SETTINGS_KEY, &app_data, sizeof(AppData));
    if (err != ESP_OK) {
      ESP_LOGE(LIGHT_TAG, "Failed to save settings %d", err);
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
      ESP_LOGE(LIGHT_TAG, "Failed to save settings %d", err);
    }
    nvs_close(my_handle);
  }
}

void light_init() {
  led_strip.access_semaphore = xSemaphoreCreateBinary();
  led_strip_init(&led_strip);

  led_strip_back.access_semaphore = xSemaphoreCreateBinary();
  led_strip_init(&led_strip_back);

  light_settings_load();

  app_data.front_params.led_strip = &led_strip;
  app_data.front_params.animation_task = NULL;
  app_data.back_params.led_strip = &led_strip_back;
  app_data.back_params.animation_task = NULL;

  animation_start(OFF, &app_data.front_params);
  animation_start(OFF, &app_data.back_params);
}

void light_set_value(uint16_t char_uuid, uint8_t* data, uint16_t len) {
  ESP_LOGI(LIGHT_TAG, "light_set_value param=%x value=%d len=%d", char_uuid, data[0], len);

  if (char_uuid == CHAR_UUID_FRONT_LIGHT_TOGGLE) {
    if (data[0] == 0) {
      animation_start(OFF, &app_data.front_params);
    } else {
      animation_start(app_data.front_params.mode, &app_data.front_params);
    }
  }

  if (char_uuid == CHAR_UUID_BACK_LIGHT_TOGGLE) {
    if (data[0] == 0) {
      animation_start(OFF, &app_data.back_params);
    } else {
      animation_start(app_data.back_params.mode, &app_data.back_params);
    }
  }

  if (char_uuid == CHAR_UUID_FRONT_LIGHT_MODE) {
    app_data.front_params.mode = data[0];
    animation_start(app_data.front_params.mode, &app_data.front_params);
  }

  if (char_uuid == CHAR_UUID_BACK_LIGHT_MODE) {
    app_data.back_params.mode = data[0];
    animation_start(app_data.back_params.mode, &app_data.back_params);
  }

  if (char_uuid == CHAR_UUID_FRONT_LIGHT_SETTING) {
    app_data.front_params.power = data[0]/100.0;
    app_data.front_params.red = data[1];
    app_data.front_params.green = data[2];
    app_data.front_params.blue = data[3];

    animation_start(app_data.front_params.mode, &app_data.front_params);
  }

  if (char_uuid == CHAR_UUID_BACK_LIGHT_SETTING) {
    app_data.back_params.power = data[0]/100.0;
    app_data.back_params.red = data[1];
    app_data.back_params.green = data[2];
    app_data.back_params.blue = data[3];

    animation_start(app_data.back_params.mode, &app_data.back_params);
  }

  if (char_uuid != CHAR_UUID_FRONT_LIGHT_TOGGLE && char_uuid != CHAR_UUID_BACK_LIGHT_TOGGLE) {
    light_settings_save();
  }

}

