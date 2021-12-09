#include "mic_params.h"
#include "stm32l475e_iot01_audio.h"

static uint16_t PCM_Buffer[PCM_BUFFER_LEN / 2];
static BSP_AUDIO_Init_t MicParams;
