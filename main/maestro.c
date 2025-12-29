#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"

#include "sdkconfig.h"
#include "esp_log.h"

/*
 This code demonstrates how to use the SPI master half duplex mode to read/write a AT932C46D EEPROM (8-bit mode).
*/

#   define ESP_HOST      SPI2_HOST
#   define PIN_NUM_MISO     19
#   define PIN_NUM_MOSI     23
#   define PIN_NUM_CLK      18
#   define PIN_NUM_CS        5
static const char TAG[] = "main";

esp_err_t ret;
spi_device_handle_t spi_handle;

void SPI_SensorConfig()
{
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));

    uint8_t tx_data[2] = {0x74, 0x27};
    trans.tx_buffer = tx_data;
    trans.rx_buffer = NULL;
    trans.length = sizeof(tx_data)*8;
    ESP_LOGI(TAG, "Transaction Size: %d bits", trans.length);

    ret = spi_device_transmit(spi_handle, &trans);
    ESP_ERROR_CHECK(ret);
}

void SPI_Transact(uint8_t *buffertoStore)
{
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));

    uint8_t tx_data[1] = {0xF7};
    trans.tx_buffer = tx_data;
    trans.rx_buffer = buffertoStore;
    trans.length = 5*8 + sizeof(tx_data)*8;
    ESP_LOGI(TAG, "Transaction Size: %d bits", trans.length);

    ret = spi_device_transmit(spi_handle, &trans);
    ESP_ERROR_CHECK(ret);
}

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

    SPI_SensorConfig();

    uint8_t RxData[5];
    while (1) {
        SPI_Transact(RxData);

        int32_t temperature = (RxData[3] << 8) | RxData[4];
        temperature = (temperature << 4) | (RxData[5] >> 4);

        ESP_LOGI(TAG, "Temperature: %d", temperature);
        ESP_LOGI(TAG, "");

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}