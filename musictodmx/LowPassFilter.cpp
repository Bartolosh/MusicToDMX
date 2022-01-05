#include "LowPassFilter.h"

static int filter_taps[LOWPASSFILTER_TAP_NUM] = {
 243,
  187,
  234,
  265,
  270,
  242,
  173,
  61,
  -93,
  -279,
  -484,
  -690,
  -873,
  -1010,
  -1075,
  -1049,
  -916,
  -669,
  -310,
  149,
  683,
  1263,
  1851,
  2406,
  2888,
  3262,
  3498,
  3579,
  3498,
  3262,
  2888,
  2406,
  1851,
  1263,
  683,
  149,
  -310,
  -669,
  -916,
  -1049,
  -1075,
  -1010,
  -873,
  -690,
  -484,
  -279,
  -93,
  61,
  173,
  242,
  270,
  265,
  234,
  187,
  243
};

void LowPassFilter_init(LowPassFilter* f) {
  int i;
  for(i = 0; i < LOWPASSFILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void LowPassFilter_put(LowPassFilter* f, int input) {
  f->history[f->last_index++] = input;
  if(f->last_index == LOWPASSFILTER_TAP_NUM)
    f->last_index = 0;
}

int LowPassFilter_get(LowPassFilter* f) {
  long long acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < LOWPASSFILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : LOWPASSFILTER_TAP_NUM-1;
    acc += (long long)f->history[index] * filter_taps[i];
  };
  return acc >> 16;
}
