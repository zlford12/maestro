#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "spi_helper.h"

static const char TAG[] = "main";
spi_device_handle_t spi_handle;

void app_main(void)
{
    SPI_BusConfig();
    SPI_SensorConfig(&spi_handle);

    uint8_t RxData[6];
    while (1) {
        SPI_Transact(spi_handle, RxData);

        int32_t temperature = (RxData[3] << 8) | RxData[4];
        temperature = (temperature << 4) | (RxData[5] >> 4);

        ESP_LOGI(TAG, "Temperature: %d", temperature);
        ESP_LOGI(TAG, "");

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}