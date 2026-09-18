#include "spi_helper.h"
#include <string.h>
#include "pins.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "driver/spi_master.h"

// SPI Config
#   define ESP_HOST         SPI3_HOST

static constexpr char TAG[] = "SPI";
esp_err_t spi_ret;

void SPI_BusConfig()
{
    // Initialize SPI bus
    ESP_LOGI(TAG, "Initializing bus SPI%d...", ESP_HOST + 1);
    spi_bus_config_t const bus_config = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    spi_ret = spi_bus_initialize(ESP_HOST, &bus_config, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(spi_ret);
}

void SPI_SensorConfig(spi_device_handle_t *handle, int cs_pin)
{
    // Initialize SPI device
    ESP_LOGI(TAG, "Adding SPI Device on GPIO%d...", cs_pin);
    spi_device_interface_config_t const dev_config = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz = 10000000,
        .duty_cycle_pos = 128,
        .mode = 0,
        .spics_io_num = cs_pin,
        .queue_size = 3
    };

    spi_ret = spi_bus_add_device(ESP_HOST, &dev_config, handle);
    ESP_ERROR_CHECK(spi_ret);

    // Configure IC-MBE for Artos DHL (BiSS-C, 40 bits)
    ESP_LOGI(TAG, "Configuring IC-MBE...");
    uint8_t tx_buf[3];
    spi_transaction_t trans = {0};
    trans.length = 24;
    trans.tx_buffer = tx_buf;

    // 1. Enable Master (Register 230 = 0xE6, set bit 0)
    tx_buf[0] = 0x02; tx_buf[1] = 0xE6; tx_buf[2] = 0x01;
    spi_ret = spi_device_transmit(*handle, &trans);
    ESP_ERROR_CHECK(spi_ret);

    // 2. Set BiSS-C mode and 5MHz clock (Register 231 = 0xE7, set bits 5:4 to 0x10)
    tx_buf[0] = 0x02; tx_buf[1] = 0xE7; tx_buf[2] = 0x20;
    spi_ret = spi_device_transmit(*handle, &trans);
    ESP_ERROR_CHECK(spi_ret);

    // 3. Set Slave 1 length to 40 bits (Register 224 = 0xE0, set value 0x27)
    tx_buf[0] = 0x02; tx_buf[1] = 0xE0; tx_buf[2] = 0x27;
    spi_ret = spi_device_transmit(*handle, &trans);
    ESP_ERROR_CHECK(spi_ret);
}

void SPI_Transact(spi_device_handle_t handle, uint8_t *buff)
{
    spi_transaction_t trans = {0};

    // 1. Trigger BiSS cycle (WriteInstruction 0x01 to address 244)
    uint8_t tx_trigger[2] = {0x07, 0x01};
    trans.tx_buffer = tx_trigger;
    trans.length = 16;
    spi_ret = spi_device_transmit(handle, &trans);
    ESP_ERROR_CHECK(spi_ret);

    // 2. Wait for cycle to complete (approx 10-25us)
    esp_rom_delay_us(25);

    // 3. Read 5 bytes of data from address 0
    // Opcode 0x03 (ReadData), Address 0x00
    uint8_t tx_read[7] = {0x03, 0x00, 0, 0, 0, 0, 0};
    uint8_t rx_read[7];
    trans.length = 56;
    trans.tx_buffer = tx_read;
    trans.rx_buffer = rx_read;
    spi_ret = spi_device_transmit(handle, &trans);
    ESP_ERROR_CHECK(spi_ret);

    // Data is in rx_read[2..6]
    memcpy(buff, &rx_read[2], 5);
}
