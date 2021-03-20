/*
 * animations.h
 *
 *  Created on: Dec 6, 2020
 *      Author: pwiklowski
 */

#ifndef MAIN_ANIMATIONS_H_
#define MAIN_ANIMATIONS_H_

#include <stdio.h>
#include "led_strip.h"
#include "animations.h"

typedef enum {
  SOLID,
  PULSE,
  SNAKE,
  CHRISTMAS,
  CHRISTMAS2,

  OFF
} Animation;

typedef struct {
  struct led_strip_t* led_strip;
  float power;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t mode;
  TaskHandle_t animation_task;
} AnimationParameters;


void animation_start(Animation anim, AnimationParameters* params);


#endif /* MAIN_ANIMATIONS_H_ */
