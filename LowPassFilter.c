#include "LowPassFilter.h"

static double filter_taps[LOWPASSFILTER_TAP_NUM] = {
  0.008343267095683986,
  0.007149583224306668,
  0.0014492517668356205,
  -0.01331060569409418,
  -0.0345042249128654,
  -0.053675798671946655,
  -0.05866861647110748,
  -0.03857434732652084,
  0.01033141819616749,
  0.08087666209830395,
  0.15569629640166371,
  0.2128725965553248,
  0.23427451265344562,
  0.2128725965553248,
  0.15569629640166371,
  0.08087666209830395,
  0.01033141819616749,
  -0.03857434732652084,
  -0.05866861647110748,
  -0.053675798671946655,
  -0.0345042249128654,
  -0.01331060569409418,
  0.0014492517668356205,
  0.007149583224306668,
  0.008343267095683986
};

void LowPassFilter_init(LowPassFilter* f) {
  int i;
  for(i = 0; i < LOWPASSFILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void LowPassFilter_put(LowPassFilter* f, double input) {
  f->history[f->last_index++] = input;
  if(f->last_index == LOWPASSFILTER_TAP_NUM)
    f->last_index = 0;
}

double LowPassFilter_get(LowPassFilter* f) {
  double acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < LOWPASSFILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : LOWPASSFILTER_TAP_NUM-1;
    acc += f->history[index] * filter_taps[i];
  };
  return acc;
}
