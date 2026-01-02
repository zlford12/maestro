#ifndef MAESTRO_SCAN_H
#define MAESTRO_SCAN_H
#include <stdint.h>

void ScanInit();
void RunScan();
void SetPulseFrequency(uint16_t new_freq);
static void PulseTimer(void *arg);

#endif //MAESTRO_SCAN_H