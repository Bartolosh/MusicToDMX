#include "LowCutFilter.h"

static int filter_taps[LOWCUTFILTER_TAP_NUM] = {
  -15716,
  -21,
  32768,
  -21,
  -15716
};

void LowCutFilter_init(LowCutFilter* f) {
  int i;
  for(i = 0; i < LOWCUTFILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void LowCutFilter_put(LowCutFilter* f, int input) {
  f->history[f->last_index++] = input;
  if(f->last_index == LOWCUTFILTER_TAP_NUM)
    f->last_index = 0;
}

int LowCutFilter_get(LowCutFilter* f) {
  long long acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < LOWCUTFILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : LOWCUTFILTER_TAP_NUM-1;
    acc += (long long)f->history[index] * filter_taps[i];
  };
  return acc >> 16;
}
