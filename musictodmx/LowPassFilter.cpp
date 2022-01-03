#include "LowPassFilter.h"

static int filter_taps[LOWPASSFILTER_TAP_NUM] = {
 -251,
  -257,
  -338,
  -387,
  -385,
  -315,
  -170,
  41,
  293,
  550,
  766,
  896,
  904,
  771,
  507,
  146,
  -250,
  -602,
  -825,
  -843,
  -606,
  -97,
  658,
  1592,
  2602,
  3569,
  4368,
  4895,
  5079,
  4895,
  4368,
  3569,
  2602,
  1592,
  658,
  -97,
  -606,
  -843,
  -825,
  -602,
  -250,
  146,
  507,
  771,
  904,
  896,
  766,
  550,
  293,
  41,
  -170,
  -315,
  -385,
  -387,
  -338,
  -257,
  -251
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
