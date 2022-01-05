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

* 60 Hz - 2550 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

*/

#define LOWCUTFILTER_TAP_NUM 5

typedef struct {
  int history[LOWCUTFILTER_TAP_NUM];
  unsigned int last_index;
} LowCutFilter;

void LowCutFilter_init(LowCutFilter* f);
void LowCutFilter_put(LowCutFilter* f, int input);
int LowCutFilter_get(LowCutFilter* f);

#endif
