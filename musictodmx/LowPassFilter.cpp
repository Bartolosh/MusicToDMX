#include "LowPassFilter.h"

static int filter_taps[LOWPASSFILTER_TAP_NUM] = {
 176,
  28,
  5,
  -41,
  -110,
  -202,
  -314,
  -442,
  -577,
  -708,
  -824,
  -911,
  -955,
  -944,
  -868,
  -720,
  -498,
  -205,
  152,
  558,
  995,
  1443,
  1877,
  2274,
  2610,
  2866,
  3026,
  3080,
  3026,
  2866,
  2610,
  2274,
  1877,
  1443,
  995,
  558,
  152,
  -205,
  -498,
  -720,
  -868,
  -944,
  -955,
  -911,
  -824,
  -708,
  -577,
  -442,
  -314,
  -202,
  -110,
  -41,
  5,
  28,
  176
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
