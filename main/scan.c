#include "scan.h"
#include <stdint.h>
#include "encoders.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"

#define LINAC_PIN 32
#define LDA_PIN 33
#define MIN_PULSE_FREQ 2
#define MAX_PULSE_FREQ 400

static constexpr char TAG[] = "scan";
const esp_timer_create_args_t pulse_timer_args = {
    .callback = &PulseTimer,
    .name = "pulse_timer"
};
esp_timer_handle_t pulse_timer;
static uint8_t gpio_state = 0;

void ScanInit()
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LINAC_PIN) | (1ULL << LDA_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    gpio_set_drive_capability(LINAC_PIN, GPIO_DRIVE_CAP_3);
    gpio_set_drive_capability(LDA_PIN, GPIO_DRIVE_CAP_3);

    ESP_ERROR_CHECK(esp_timer_create(&pulse_timer_args, &pulse_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(pulse_timer, 1000000 / (2 * MIN_PULSE_FREQ)));
}

void RunScan()
{
    uint8_t RxData[6];

    ESP_LOGI(TAG, "Running scan");
    ReadEncoders(RxData);

    int32_t temperature = (RxData[3] << 8) | RxData[4];
    temperature = (temperature << 4) | (RxData[5] >> 4);

    ESP_LOGI(TAG, "Temperature: %d \n", temperature);
}

void SetPulseFrequency(uint16_t new_freq)
{
    if (new_freq < MAX_PULSE_FREQ && new_freq > MIN_PULSE_FREQ)
    {
        ESP_ERROR_CHECK(esp_timer_stop(pulse_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(pulse_timer, 1000000 / (2 * new_freq)));
        ESP_LOGI(TAG, "Pulse frequency updated to %d Hz", new_freq);
    }
    else
    {
        ESP_LOGI(TAG, "Pulse frequency %d Hz is out of range (%d - %d Hz)", new_freq, MIN_PULSE_FREQ, MAX_PULSE_FREQ);
    }
}

static void PulseTimer(void *arg)
{
    gpio_state = !gpio_state;
    gpio_set_level(LINAC_PIN, gpio_state);
    gpio_set_level(LDA_PIN, gpio_state);

    if (gpio_state)
    {
        ESP_LOGI(TAG, "Pulse rising edge");
    }
}


