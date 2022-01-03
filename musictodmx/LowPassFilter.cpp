#include "LowPassFilter.h"

static int filter_taps[LOWPASSFILTER_TAP_NUM] = {
 189,
  65,
  61,
  42,
  8,
  -43,
  -113,
  -198,
  -297,
  -403,
  -510,
  -610,
  -692,
  -748,
  -767,
  -741,
  -663,
  -529,
  -336,
  -88,
  211,
  551,
  921,
  1304,
  1686,
  2047,
  2371,
  2642,
  2846,
  2973,
  3016,
  2973,
  2846,
  2642,
  2371,
  2047,
  1686,
  1304,
  921,
  551,
  211,
  -88,
  -336,
  -529,
  -663,
  -741,
  -767,
  -748,
  -692,
  -610,
  -510,
  -403,
  -297,
  -198,
  -113,
  -43,
  8,
  42,
  61,
  65,
  189
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
