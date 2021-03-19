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

#define SERVICE_UUID_LIGHT_FRONT      0x00FF
#define CHAR_UUID_LIGHT_TOGGLE        0xFF01
#define CHAR_UUID_LIGHT_MODE          0xFF02
#define CHAR_UUID_LIGHT_SETTING       0xFF03

#define LED_STRIP_LENGTH 10
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



void light_init() {
  led_strip.access_semaphore = xSemaphoreCreateBinary();
  led_strip_init(&led_strip);

  //TODO read it from flash
  front_params.blue = 0xFF;
  front_params.red = 0;
  front_params.green = 0;
  front_params.power = 1;
  front_params.led_strip = &led_strip;
  front_params.mode = SNAKE;

  animation_start(front_params.mode, &front_params);
}

void light_set_value(uint16_t char_uuid, uint8_t* data, uint16_t len) {
  ESP_LOGI(LIGHT_TAG, "light_set_value param=%x value=%d len=%d", char_uuid, data[0], len);

  if (char_uuid == CHAR_UUID_LIGHT_TOGGLE) {
    if (data[0] == 0) {
      animation_start(OFF, &front_params);
    } else {
      animation_start(front_params.mode, &front_params);
    }
  }

  if (char_uuid == CHAR_UUID_LIGHT_MODE) {
    front_params.mode = data[0];
  }

  // TODO store it to flash
}

