#include "main.h"
#include "nvs_flash.h"
#include "esp_system.h"

#include "light.h"
#include "battery.h"

#include "service_battery.h"


AppData app_data;

void app_main(void)
{
    esp_err_t ret;

    /* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    light_init();

    ble_init();

    battery_init();
    battery_start();
}
