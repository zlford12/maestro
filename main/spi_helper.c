#include "spi_helper.h"
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"

static const char TAG[] = "SPI";
esp_err_t ret;

void SPI_SensorConfig(spi_device_handle_t handle)
{
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));

    uint8_t tx_data[2] = {0x74, 0x27};
    trans.tx_buffer = tx_data;
    trans.rx_buffer = NULL;
    trans.length = sizeof(tx_data)*8;
    ESP_LOGI(TAG, "Transaction Size: %d bits", trans.length);

    ret = spi_device_transmit(handle, &trans);
    ESP_ERROR_CHECK(ret);
}

void SPI_Transact(spi_device_handle_t handle, uint8_t *buffertoStore)
{
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));

    uint8_t tx_data[1] = {0xF7};
    trans.tx_buffer = tx_data;
    trans.rx_buffer = buffertoStore;
    trans.length = 5*8 + sizeof(tx_data)*8;
    ESP_LOGI(TAG, "Transaction Size: %d bits", trans.length);

    ret = spi_device_transmit(handle, &trans);
    ESP_ERROR_CHECK(ret);
}
