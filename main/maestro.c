#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encoders.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "nvs.h"
#include "spi_helper.h"
#include "wifi_helper.h"

static constexpr char TAG[] = "main";

void app_main(void)
{
    SPI_BusConfig();
    EncoderInit();
    WifiConfig();

    uint8_t RxData[6];

    while (1) {
        if (!WifiConnected())
        {
            WifiConnect();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        ReadEncoders(RxData);

        int32_t temperature = (RxData[3] << 8) | RxData[4];
        temperature = (temperature << 4) | (RxData[5] >> 4);

        ESP_LOGI(TAG, "Temperature: %d", temperature);
        ESP_LOGI(TAG, "");

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}