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

#   define ESP_HOST      SPI2_HOST
#   define PIN_NUM_MISO     19
#   define PIN_NUM_MOSI     23
#   define PIN_NUM_CLK      18
#   define PIN_NUM_CS        5

esp_err_t ret;
spi_device_handle_t spi_handle;

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing bus SPI%d...", ESP_HOST + 1);
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz = 10000000,
        .duty_cycle_pos = 128,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 3
    };

    //Initialize the SPI bus
    ret = spi_bus_initialize(ESP_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    ret = spi_bus_add_device(ESP_HOST, &devcfg, &spi_handle);
    ESP_ERROR_CHECK(ret);

    SPI_SensorConfig(spi_handle);

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