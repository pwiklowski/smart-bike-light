#ifndef service_front_light_h
#define service_front_light_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    IDX_SVC,
    IDX_CHAR_FRONT_LIGHT_TOGGLE,
    IDX_CHAR_VAL_FRONT_LIGHT_TOGGLE,

    IDX_CHAR_FRONT_LIGHT_MODE,
    IDX_CHAR_VAL_FRONT_LIGHT_MODE,

    IDX_CHAR_FRONT_LIGHT_SETTING,
    IDX_CHAR_VAL_FRONT_LIGHT_SETTING,

    IDX_CHAR_BACK_LIGHT_TOGGLE,
    IDX_CHAR_VAL_BACK_LIGHT_TOGGLE,

    IDX_CHAR_BACK_LIGHT_MODE,
    IDX_CHAR_VAL_BACK_LIGHT_MODE,

    IDX_CHAR_BACK_LIGHT_SETTING,
    IDX_CHAR_VAL_BACK_LIGHT_SETTING,

    HRS_IDX_NB,
};

void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

#endif
