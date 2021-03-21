/*
 * light.h
 *
 *  Created on: Mar 19, 2021
 *      Author: pwiklowski
 */

#ifndef MAIN_LIGHT_H_
#define MAIN_LIGHT_H_

#include "animations.h"

typedef struct {
  AnimationParameters front_params;
  AnimationParameters back_params;

} AppData;

void light_init();
void light_set_value(uint16_t char_uuid, uint8_t* data, uint16_t len);

#endif /* MAIN_LIGHT_H_ */
