#ifndef LOWCUTFILTER_H_
#define LOWCUTFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 5100 Hz

fixed point precision: 16 bits

* 0 Hz - 50 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

* 60 Hz - 500 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

*/

#include <stdint.h>

#define LOWCUTFILTER_TAP_NUM 5

typedef struct {
  int32_t history[LOWCUTFILTER_TAP_NUM];
  uint32_t last_index;
} LowCutFilter;

void LowCutFilter_init(LowCutFilter* f);
void LowCutFilter_put(LowCutFilter* f, int32_t input);
int32_t LowCutFilter_get(LowCutFilter* f);

#endif
