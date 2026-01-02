#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encoders.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "nvs.h"
#include "scan.h"
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
    ScanInit();

    while (1) {
        if (!WifiConnected())
        {
            WifiConnect();
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }

        char cmd[64] = "";
        SocketListen(cmd, sizeof(cmd));
        ESP_LOGI(TAG, "Command: %s", cmd);
        if (strcmp(cmd, "scan") == 0)
        {
            RunScan();
        }
        else if (strncmp(cmd, "setfreq ", 8) == 0)
        {
            uint16_t new_freq = atoi(cmd + 8);
            if (new_freq > 0)
            {
                SetPulseFrequency(new_freq);
            }
            else
            {
                ESP_LOGI(TAG, "Invalid frequency value");
            }
        }
        else
        {
            ESP_LOGI(TAG, "Unknown command");
            continue;
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}