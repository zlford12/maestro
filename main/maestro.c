#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encoders.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "nvs.h"
#include "socket_helper.h"
#include "spi_helper.h"
#include "wifi_helper.h"

static constexpr char TAG[] = "main";

void app_main(void)
{
    SPI_BusConfig();
    EncoderInit();
    WifiConfig();
    SocketInit();

    uint8_t RxData[6];

    while (1) {
        char cmd[64] = "";
        if (!WifiConnected())
        {
            WifiConnect();
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }

        SocketListen(cmd, sizeof(cmd));
        ESP_LOGI(TAG, "Command: %s", cmd);

        ReadEncoders(RxData);

        int32_t temperature = (RxData[3] << 8) | RxData[4];
        temperature = (temperature << 4) | (RxData[5] >> 4);

        ESP_LOGI(TAG, "Temperature: %d \n", temperature);

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}