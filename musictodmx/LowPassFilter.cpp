#include "LowPassFilter.h"

static int filter_taps[LOWPASSFILTER_TAP_NUM] = {
  165,
  -15,
  -68,
  -158,
  -282,
  -436,
  -608,
  -783,
  -940,
  -1057,
  -1110,
  -1079,
  -947,
  -706,
  -355,
  95,
  622,
  1197,
  1783,
  2338,
  2822,
  3197,
  3436,
  3517,
  3436,
  3197,
  2822,
  2338,
  1783,
  1197,
  622,
  95,
  -355,
  -706,
  -947,
  -1079,
  -1110,
  -1057,
  -940,
  -783,
  -608,
  -436,
  -282,
  -158,
  -68,
  -15,
  165
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
