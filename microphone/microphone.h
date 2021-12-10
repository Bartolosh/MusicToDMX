#include "stm32l475e_iot01_audio.h"

void init_buffer(int16_t *BUFFER);

void target_audio_buffer_full();
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance);
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance);
void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance);
void print_stats();
void start_recording();
