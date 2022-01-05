#include "LowPassFilter.h"

static int filter_taps[LOWPASSFILTER_TAP_NUM] = {
   171,
  13,
  -10,
  -50,
  -108,
  -185,
  -278,
  -384,
  -497,
  -610,
  -715,
  -804,
  -864,
  -888,
  -867,
  -793,
  -663,
  -475,
  -231,
  63,
  398,
  762,
  1141,
  1518,
  1875,
  2195,
  2463,
  2665,
  2790,
  2833,
  2790,
  2665,
  2463,
  2195,
  1875,
  1518,
  1141,
  762,
  398,
  63,
  -231,
  -475,
  -663,
  -793,
  -867,
  -888,
  -864,
  -804,
  -715,
  -610,
  -497,
  -384,
  -278,
  -185,
  -108,
  -50,
  -10,
  13,
  171
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
