#include "scan.h"

#include <stdint.h>

#include "encoders.h"
#include "esp_log.h"

static constexpr char TAG[] = "scan";


void RunScan()
{
    uint8_t RxData[6];

    ESP_LOGI(TAG, "Running scan");
    ReadEncoders(RxData);

    int32_t temperature = (RxData[3] << 8) | RxData[4];
    temperature = (temperature << 4) | (RxData[5] >> 4);

    ESP_LOGI(TAG, "Temperature: %d \n", temperature);
}
