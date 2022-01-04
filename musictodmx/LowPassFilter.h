#ifndef LOWPASSFILTER_H_
#define LOWPASSFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 4000 Hz

fixed point precision: 16 bits

* 0 Hz - 350 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 450 Hz - 2000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define LOWPASSFILTER_TAP_NUM 57

typedef struct {
  int history[LOWPASSFILTER_TAP_NUM];
  unsigned int last_index;
} LowPassFilter;

void LowPassFilter_init(LowPassFilter* f);
void LowPassFilter_put(LowPassFilter* f, int input);
int LowPassFilter_get(LowPassFilter* f);

#endif
