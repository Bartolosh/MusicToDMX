#ifndef LOWPASSFILTER_H_
#define LOWPASSFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 4000 Hz

fixed point32_t precision: 16 bits

* 0 Hz - 300 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 400 Hz - 2050 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#include <stdint.h>

#define LOWPASSFILTER_TAP_NUM 23

typedef struct {
  int32_t history[LOWPASSFILTER_TAP_NUM];
  int16_t last_index;
} LowPassFilter;

void LowPassFilter_init(LowPassFilter* f);
void LowPassFilter_put(LowPassFilter* f, int16_t input);
int16_t LowPassFilter_get(LowPassFilter* f);

#endif
