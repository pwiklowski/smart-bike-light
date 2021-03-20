#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include <led_strip.h>
#include "animations.h"
#include "esp_log.h"

AnimationParameters front_params;
AnimationParameters back_params;

#define LIGHT_TAG "LIGHT"

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


void light_init() {
  led_strip.access_semaphore = xSemaphoreCreateBinary();
  led_strip_init(&led_strip);

  led_strip_back.access_semaphore = xSemaphoreCreateBinary();
  led_strip_init(&led_strip_back);

  //TODO read it from flash
  front_params.blue = 0xFF;
  front_params.red = 0;
  front_params.green = 0;
  front_params.power = 1;
  front_params.led_strip = &led_strip;
  front_params.mode = SNAKE;
  animation_start(front_params.mode, &front_params);

  back_params.blue = 0;
  back_params.red = 0;
  back_params.green = 0xFF;
  back_params.power = 1;
  back_params.led_strip = &led_strip_back;
  back_params.mode = PULSE;
  animation_start(back_params.mode, &back_params);
}

void light_set_value(uint16_t char_uuid, uint8_t* data, uint16_t len) {
  ESP_LOGI(LIGHT_TAG, "light_set_value param=%x value=%d len=%d", char_uuid, data[0], len);

  if (char_uuid == CHAR_UUID_FRONT_LIGHT_TOGGLE) {
    if (data[0] == 0) {
      animation_start(OFF, &front_params);
    } else {
      animation_start(front_params.mode, &front_params);
    }
  }

  if (char_uuid == CHAR_UUID_BACK_LIGHT_TOGGLE) {
    if (data[0] == 0) {
      animation_start(OFF, &back_params);
    } else {
      animation_start(back_params.mode, &back_params);
    }
  }

  if (char_uuid == CHAR_UUID_FRONT_LIGHT_MODE) {
    front_params.mode = data[0];
      animation_start(front_params.mode, &front_params);
  }

  if (char_uuid == CHAR_UUID_BACK_LIGHT_MODE) {
    back_params.mode = data[0];
    animation_start(back_params.mode, &back_params);
  }

  if (char_uuid == CHAR_UUID_FRONT_LIGHT_SETTING) {
    front_params.power = data[0]/100.0;
    front_params.red = data[1];
    front_params.green = data[2];
    front_params.blue = data[3];

    animation_start(front_params.mode, &front_params);
  }

  if (char_uuid == CHAR_UUID_BACK_LIGHT_SETTING) {
    back_params.power = data[0]/100.0;
    back_params.red = data[1];
    back_params.green = data[2];
    back_params.blue = data[3];

    animation_start(back_params.mode, &back_params);
  }

  // TODO store it to flash
}

