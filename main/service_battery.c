#include "esp_bt.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include "service_battery.h"
#include "gatt.h"

static char* GATTS_TABLE_TAG = "BatteryService";

extern struct gatts_profile_inst gl_profile_tab[PROFILE_NUM];

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE 1024
#define CHAR_DECLARATION_SIZE (sizeof(uint8_t))

static uint8_t adv_config_done = 0;

uint16_t battery_handle_table[BATTERY_IDX_NB];

uint16_t battery_notification_table[BATTERY_IDX_NB];

static uint16_t connection_id;

bool is_connected = false;

uint8_t battery_service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0xfc, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

/* The length of adv data must be less than 31 bytes */
esp_ble_adv_data_t battery_adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, // slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, // test_manufacturer,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(battery_service_uuid),
    .p_service_uuid = battery_service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
esp_ble_adv_data_t battery_scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(battery_service_uuid),
    .p_service_uuid = battery_service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

extern esp_ble_adv_params_t adv_params;

static const uint16_t GATTS_SERVICE_UUID_BATTERY = 0x00FE;

static const uint16_t GATTS_CHAR_UUID_VOLTAGE = 0xFE01;

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t config_descriptor[2] = {0x00, 0x00};


static uint8_t battery;

/* Full Database Description - Used to add attributes into the database */
static const esp_gatts_attr_db_t gatt_db[BATTERY_IDX_NB] = {
    // Service Declaration
    [IDX_SVC_BATTERY] = {{ESP_GATT_AUTO_RSP},
                         {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t),
                          sizeof(GATTS_SERVICE_UUID_BATTERY), (uint8_t *)&GATTS_SERVICE_UUID_BATTERY}},

    /* Characteristic Declaration */
    [IDX_CHAR_VOLTAGE] = {{ESP_GATT_AUTO_RSP},
                          {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},
    /* Characteristic Value */
    [IDX_CHAR_VAL_VOLTAGE] = {{ESP_GATT_AUTO_RSP},
                              {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_VOLTAGE,
                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX,
                               sizeof(uint16_t), (uint8_t*)&battery }},
    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_VOLTAGE] = {{ESP_GATT_AUTO_RSP},
                              {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid,
                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(uint16_t), sizeof(config_descriptor),
                               (uint8_t *)config_descriptor}},


};

void gatts_battery_service_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
  switch (event) {
  case ESP_GATTS_REG_EVT: {
    esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(SAMPLE_DEVICE_NAME);
    if (set_dev_name_ret) {
      ESP_LOGE(GATTS_TABLE_TAG, "set device name failed, error code = %x", set_dev_name_ret);
    }
    // config adv data
    esp_err_t ret = esp_ble_gap_config_adv_data(&battery_adv_data);
    if (ret) {
      ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
    }
    adv_config_done |= ADV_CONFIG_FLAG;

    adv_config_done |= SCAN_RSP_CONFIG_FLAG;
    esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, BATTERY_IDX_NB, SERVICE_BATTERY_ID);
    if (create_attr_ret) {
      ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
    }

    gl_profile_tab[SERVICE_BATTERY_ID].gatts_if = gatts_if;

  }
  break;
  case ESP_GATTS_READ_EVT:
    ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_READ_EVT");
    break;
  case ESP_GATTS_WRITE_EVT:
    ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_WRITE_EVT");
    ESP_LOGI(GATTS_TABLE_TAG, "GATT_WRITE_EVT, handle = %d, value len = %d, value :", param->write.handle,
             param->write.len);
    esp_log_buffer_hex(GATTS_TABLE_TAG, battery_handle_table, 20);

    if (!param->write.is_prep) {
      ESP_LOGI(GATTS_TABLE_TAG, "prepare write not needed %d %d", battery_handle_table[IDX_CHAR_CFG_VOLTAGE],
               param->write.handle);
      uint16_t descr_value = param->write.value[1] << 8 | param->write.value[0];

      uint16_t index = 0;

      for (uint8_t i = 0; i < BATTERY_IDX_NB; i++) {
        if (battery_handle_table[i] == param->write.handle) {
          index = i;
          break;
        }
      }

      battery_notification_table[index] = descr_value;
      ESP_LOGI(GATTS_TABLE_TAG, "notify enable %d %d %d ", index, param->write.handle, descr_value);

//      if (index == IDX_CHAR_CFG_VOLTAGE) {
//        battery_update_value(state.voltage.value, IDX_CHAR_VAL_VOLTAGE, true);
//      } else if (index == IDX_CHAR_CFG_CURRENT) {
//        battery_update_value(state.current.value, IDX_CHAR_VAL_CURRENT, true);
//      } else if (index == IDX_CHAR_CFG_USED_ENERGY) {
//        battery_update_value(state.used_energy.value, IDX_CHAR_VAL_USED_ENERGY, true);
//      } else if (index == IDX_CHAR_CFG_TOTAL_ENERGY) {
//        battery_update_value(state.total_energy.value, IDX_CHAR_VAL_TOTAL_ENERGY, true);
//      }
    }
    break;
  case ESP_GATTS_EXEC_WRITE_EVT:
    ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
    break;
  case ESP_GATTS_MTU_EVT:
    ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
    break;
  case ESP_GATTS_CONF_EVT:
    break;
  case ESP_GATTS_START_EVT:
    ESP_LOGI(GATTS_TABLE_TAG, "SERVICE_START_EVT, status %d, service_handle %d", param->start.status,
             param->start.service_handle);
    break;
  case ESP_GATTS_CONNECT_EVT:
    ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d", param->connect.conn_id);
    connection_id = param->connect.conn_id;
    is_connected = true;
    break;
  case ESP_GATTS_DISCONNECT_EVT:
    ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
    esp_ble_gap_start_advertising(&adv_params);
    is_connected = false;
    break;
  case ESP_GATTS_CREAT_ATTR_TAB_EVT: {
    if (param->add_attr_tab.status != ESP_GATT_OK) {
      ESP_LOGE(GATTS_TABLE_TAG, "create attribute table failed, error code=0x%x", param->add_attr_tab.status);
    } else if (param->add_attr_tab.num_handle != BATTERY_IDX_NB) {
      ESP_LOGE(GATTS_TABLE_TAG, "create attribute table abnormally, num_handle (%d) \
                        doesn't equal to HRS_IDX_NB(%d)",
               param->add_attr_tab.num_handle, BATTERY_IDX_NB);
    } else {
      ESP_LOGI(GATTS_TABLE_TAG, "create attribute table successfully, the number handle = %d\n",
               param->add_attr_tab.num_handle);
      memcpy(battery_handle_table, param->add_attr_tab.handles, sizeof(battery_handle_table));
      esp_ble_gatts_start_service(battery_handle_table[IDX_SVC_BATTERY]);
    }
    break;
  }
  case ESP_GATTS_STOP_EVT:
  case ESP_GATTS_OPEN_EVT:
  case ESP_GATTS_CANCEL_OPEN_EVT:
  case ESP_GATTS_CLOSE_EVT:
  case ESP_GATTS_LISTEN_EVT:
  case ESP_GATTS_CONGEST_EVT:
  case ESP_GATTS_UNREG_EVT:
  case ESP_GATTS_DELETE_EVT:
  default:
    break;
  }
}

//void battery_update_value(double value, uint16_t characteristic_index, bool force_notify) {
//  bool was_changed = false;
//
//
//
//  esp_ble_gatts_set_attr_value(battery_handle_table[characteristic_index], sizeof(characteristic.bytes), (uint8_t *)characteristic.bytes);
//
//  if (battery_notification_table[characteristic_index + 1] == 0x0001 && (was_changed || force_notify)) {
//
//    esp_ble_gatts_send_indicate(battery_profile_tab.gatts_if, connection_id, battery_handle_table[characteristic_index],
//                                sizeof(characteristic.bytes), (uint8_t *)characteristic.bytes, false);
//  }
//}
