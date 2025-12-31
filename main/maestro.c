#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "nvs.h"
#include "spi_helper.h"
#include "wifi_helper.h"

#define T_ENC_CS       4
#define R_ENC_CS       5
#define S_ENC_CS       3
#define D_ENC_CS       2

static constexpr char TAG[] = "main";
spi_device_handle_t t_enc_handle, r_enc_handle, s_enc_handle, d_enc_handle;

void app_main(void)
{
    SPI_BusConfig();
    SPI_SensorConfig(&t_enc_handle, T_ENC_CS);
    //SPI_SensorConfig(&r_enc_handle, R_ENC_CS);
    //SPI_SensorConfig(&s_enc_handle, S_ENC_CS);
    //SPI_SensorConfig(&d_enc_handle, D_ENC_CS);

    WifiConfig();

    uint8_t RxData[6];

    while (1) {
        if (!WifiConnected())
        {
            WifiConnect();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        SPI_Transact(t_enc_handle, RxData);

        int32_t temperature = (RxData[3] << 8) | RxData[4];
        temperature = (temperature << 4) | (RxData[5] >> 4);

        ESP_LOGI(TAG, "Temperature: %d", temperature);
        ESP_LOGI(TAG, "");

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}