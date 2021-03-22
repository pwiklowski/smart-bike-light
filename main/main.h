/*
 * main.h
 *
 *  Created on: Mar 22, 2021
 *      Author: pwiklowski
 */

#ifndef MAIN_MAIN_H_
#define MAIN_MAIN_H_

#include "animations.h"


typedef struct {
  AnimationParameters front_params;
  AnimationParameters back_params;

  uint8_t battery_level;
} AppData;


#endif /* MAIN_MAIN_H_ */
