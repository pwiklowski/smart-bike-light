#ifndef service_battery_h
#define service_battery_h

#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include "esp_log.h"
#include <string.h>
#include "gatt.h"

enum {
  IDX_SVC_BATTERY,

  IDX_CHAR_VOLTAGE,
  IDX_CHAR_VAL_VOLTAGE,
  IDX_CHAR_CFG_VOLTAGE,

  BATTERY_IDX_NB,
};

void gatts_battery_service_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void battery_update_value(uint8_t value, uint16_t characteristic_index);
void state_update();


#endif
