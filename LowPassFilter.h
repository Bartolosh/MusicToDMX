#ifndef LOWPASSFILTER_H_
#define LOWPASSFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 2000 Hz

* 0 Hz - 200 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.6987047106904303 dB

* 300 Hz - 1000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -41.01892043065909 dB

*/

#define LOWPASSFILTER_TAP_NUM 25

typedef struct {
  double history[LOWPASSFILTER_TAP_NUM];
  unsigned int last_index;
} LowPassFilter;

void LowPassFilter_init(LowPassFilter* f);
void LowPassFilter_put(LowPassFilter* f, double input);
double LowPassFilter_get(LowPassFilter* f);

#endif