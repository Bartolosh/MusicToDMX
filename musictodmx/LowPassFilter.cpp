#include "LowPassFilter.h"

static int filter_taps[LOWPASSFILTER_TAP_NUM] = {
 235,
  193,
  249,
  295,
  322,
  319,
  280,
  199,
  76,
  -85,
  -275,
  -479,
  -678,
  -850,
  -972,
  -1022,
  -981,
  -837,
  -583,
  -225,
  224,
  743,
  1300,
  1862,
  2389,
  2846,
  3199,
  3422,
  3498,
  3422,
  3199,
  2846,
  2389,
  1862,
  1300,
  743,
  224,
  -225,
  -583,
  -837,
  -981,
  -1022,
  -972,
  -850,
  -678,
  -479,
  -275,
  -85,
  76,
  199,
  280,
  319,
  322,
  295,
  249,
  193,
  235
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
