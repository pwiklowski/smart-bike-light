#include "animations.h"
#include "freertos/FreeRTOS.h"
#include "esp_task.h"
#include <string.h>
#include "led_strip.h"

TaskHandle_t animation_task = NULL;

void animation_snake(void *arg) {
  AnimationParameters *params = (struct AnimationParameters*) arg;

  while (1) {
    for (int j = 0; j < params->led_strip->led_strip_length; j++) {
      for (int i = 0; i < params->led_strip->led_strip_length; i++) {
        uint8_t r = params->red;
        uint8_t g = params->green;
        uint8_t b = params->blue;

        float p = i == j ? params->power : 0;

        led_strip_set_pixel_rgb(params->led_strip, i, p * r, p * g, p * b);
      }
      led_strip_show(params->led_strip);
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }
}

void animation_pulse(void *arg) {
  AnimationParameters *params = (struct AnimationParameters*) arg;
  uint8_t steps = 50;

  while (1) {
    for (int step = 0; step < steps; step++) {
      for (int i = 0; i < params->led_strip->led_strip_length; i++) {
        uint8_t r = params->red;
        uint8_t g = params->green;
        uint8_t b = params->blue;

        float p = params->power * ((float) step / steps);

        led_strip_set_pixel_rgb(params->led_strip, i, p * r, p * g, p * b);
      }
      led_strip_show(params->led_strip);
      vTaskDelay(30 / portTICK_PERIOD_MS);
    }
    for (int step = steps; step >= 0; step--) {
      for (int i = 0; i < params->led_strip->led_strip_length; i++) {
        uint8_t r = params->red;
        uint8_t g = params->green;
        uint8_t b = params->blue;

        float p = params->power * ((float) step / steps);

        led_strip_set_pixel_rgb(params->led_strip, i, p * r, p * g, p * b);
      }
      led_strip_show(params->led_strip);
      vTaskDelay(30 / portTICK_PERIOD_MS);
    }
  }
}

void animation_christmas(void *arg) {
  AnimationParameters *params = (struct AnimationParameters*) arg;
  while (1) {
    for (int j = 0; j < 3; j++) {
      for (int i = 0; i < params->led_strip->led_strip_length; i++) {
        uint8_t r = (i + j) % 3 == 0 ? 255 : 0;
        uint8_t g = (i + j) % 3 == 1 ? 255 : 0;
        uint8_t b = (i + j) % 3 == 2 ? 255 : 0;

        float p = params->power;

        led_strip_set_pixel_rgb(params->led_strip, i, p * r, p * g, p * b);
      }
      led_strip_show(params->led_strip);
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
}

void animation_christmas_2(void *arg) {
  AnimationParameters *params = (struct AnimationParameters*) arg;

  while (1) {
    for (int j = 0; j < 5; j++) {
      for (int i = 0; i < params->led_strip->led_strip_length; i++) {
        uint8_t r = (i + j) % 5 == 0 ? 255 : 0;
        uint8_t g = (i + j) % 5 == 0 ? 0 : 255;
        uint8_t b = 0;

        float p = params->power;

        led_strip_set_pixel_rgb(params->led_strip, i, p * r, p * g, p * b);
      }
      led_strip_show(params->led_strip);
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
}

void animation_set_solid_color(struct led_strip_t *led_strip, float power, uint8_t red, uint8_t green, uint8_t blue) {
  for (int i = 0; i < led_strip->led_strip_length; i++) {
    led_strip_set_pixel_rgb(led_strip, i, power * red, power * green, power * blue);
  }
  led_strip_show(led_strip);
}

void animation_start(Animation anim, AnimationParameters* params) {

  if (animation_task != NULL) {
    vTaskDelete(animation_task);
    animation_task = NULL;
  }

  switch (anim) {
  case SOLID:
    animation_set_solid_color(params->led_strip, 1, 0xFF, 0, 0);
    break;
  case SNAKE:
    xTaskCreate(animation_snake, "animation_snake", 1024, params, ESP_TASK_MAIN_PRIO + 1, &animation_task);
    break;
  case PULSE:
    xTaskCreate(animation_pulse, "animation_pulse", 1024, params, ESP_TASK_MAIN_PRIO + 1, &animation_task);
    break;
  case CHRISTMAS:
    xTaskCreate(animation_christmas, "animation_christmas", 1024, params, ESP_TASK_MAIN_PRIO + 1, &animation_task);
    break;
  case CHRISTMAS2:
    xTaskCreate(animation_christmas_2, "animation_christmas_2", 1024, params, ESP_TASK_MAIN_PRIO + 1, &animation_task);
    break;
  default:
    animation_set_solid_color(params->led_strip, 0, 0, 0, 0);
    break;
  }
}
