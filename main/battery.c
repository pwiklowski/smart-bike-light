/*
 * battery.c
 *
 *  Created on: Mar 21, 2021
 *      Author: pwiklowski
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

static const float DIVIDER = 1.0+3.92/1.0; //TODO add calibration data

void battery_init() {
  if (unit == ADC_UNIT_1) {
      adc1_config_width(width);
      adc1_config_channel_atten(channel, atten);
  } else {
      adc2_config_channel_atten((adc2_channel_t)channel, atten);
  }

  //Characterize ADC
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
}

void battery_read() {
  uint32_t adc_reading = 0;
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
      if (unit == ADC_UNIT_1) {
          adc_reading += adc1_get_raw((adc1_channel_t)channel);
      } else {
          int raw;
          adc2_get_raw((adc2_channel_t)channel, width, &raw);
          adc_reading += raw;
      }
  }
  adc_reading /= NO_OF_SAMPLES;
  //Convert adc_reading to voltage in mV
  float voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
  printf("Raw: %d\tVoltage: %fmV\n", adc_reading, voltage* DIVIDER);
}


void battery_task(){
  while(1) {
    battery_read();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void battery_start() {
  xTaskCreate(battery_task, "battery_task", 1024, NULL, ESP_TASK_MAIN_PRIO + 1, NULL);
}
