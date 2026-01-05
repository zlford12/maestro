#include "scan.h"
#include <stdint.h>
#include "pins.h"
#include "encoders.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "socket_helper.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MIN_PULSE_FREQ 2
#define MAX_PULSE_FREQ 400
#define MAX_LINE_SIZE 256
#define MAX_POSITION 2100

static constexpr char TAG[] = "scan";
const esp_timer_create_args_t pulse_timer_args = {
    .callback = &PulseTimer,
    .name = "pulse_timer"
};
esp_timer_handle_t pulse_timer;
static uint8_t pulse_state = 0;
static uint32_t line_buffer[MAX_LINE_SIZE];
static uint32_t line_buffer_size = 0;
bool read_encoders = false;
static TaskHandle_t scan_task_hdl;

void ScanInit()
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LINAC_PIN) | (1ULL << LDA_TRIGGER);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    gpio_set_drive_capability(LINAC_PIN, GPIO_DRIVE_CAP_3);
    gpio_set_drive_capability(LDA_TRIGGER, GPIO_DRIVE_CAP_3);

    ESP_ERROR_CHECK(esp_timer_create(&pulse_timer_args, &pulse_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(pulse_timer, 1000000 / (2 * MIN_PULSE_FREQ)));
}

void RunScan()
{
    ESP_LOGI(TAG, "Running scan");

    scan_task_hdl = xTaskGetCurrentTaskHandle();
    xTaskNotifyStateClear(scan_task_hdl);
    line_buffer_size = 0;
    read_encoders = true;
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

    ESP_LOGI(TAG, "Scan finished, %d frames captured", line_buffer_size);
    SendFrames(line_buffer, line_buffer_size * sizeof(uint32_t));
}

void SetPulseFrequency(uint16_t new_freq)
{
    if (new_freq < MAX_PULSE_FREQ && new_freq > MIN_PULSE_FREQ)
    {
        ESP_ERROR_CHECK(esp_timer_stop(pulse_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(pulse_timer, 1000000 / (2 * new_freq)));
        ESP_LOGI(TAG, "Pulse frequency updated to %d Hz", new_freq);

        char freq_str[16];
        sprintf(freq_str, "%d", new_freq);
        SendResponse(freq_str);
    }
    else
    {
        ESP_LOGI(TAG, "Pulse frequency %d Hz is out of range (%d - %d Hz)", new_freq, MIN_PULSE_FREQ, MAX_PULSE_FREQ);
        SendResponse("invalid");
    }
}

static void PulseTimer(void *arg)
{
    pulse_state = !pulse_state;
    bool acquire = pulse_state && read_encoders;
    gpio_set_level(LINAC_PIN, pulse_state);
    gpio_set_level(LDA_TRIGGER, acquire);

    if (acquire)
    {
        uint8_t RxData[6];
        ReadEncoders(RxData);
        int32_t position = (RxData[3] << 8) | RxData[4];
        line_buffer[line_buffer_size] = (position << 4) | (RxData[5] >> 4);
        //ESP_LOGI(TAG, "Temperature: %d \n", line_buffer[line_buffer_size]);

        if (line_buffer_size == MAX_LINE_SIZE - 1 || line_buffer[line_buffer_size] > MAX_POSITION)
        {
            read_encoders = false;
            if (scan_task_hdl != NULL)
            {
                xTaskNotify(scan_task_hdl, 0, eNoAction);
            }
        }
        else
        {
            line_buffer_size++;
        }
    }
}


