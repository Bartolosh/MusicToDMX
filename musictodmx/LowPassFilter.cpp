#include "LowPassFilter.h"

static int filter_taps[LOWPASSFILTER_TAP_NUM] = {
 331,
  511,
  815,
  1145,
  1450,
  1674,
  1767,
  1700,
  1474,
  1133,
  753,
  440,
  304,
  438,
  900,
  1690,
  2744,
  3945,
  5133,
  6140,
  6814,
  7051,
  6814,
  6140,
  5133,
  3945,
  2744,
  1690,
  900,
  438,
  304,
  440,
  753,
  1133,
  1474,
  1700,
  1767,
  1674,
  1450,
  1145,
  815,
  511,
  331
};

void LowPassFilter_init(L50owPassFilter* f) {
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
