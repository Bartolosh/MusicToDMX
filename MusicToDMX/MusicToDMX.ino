#include <STM32FreeRTOS.h>
#include <timers.h>
#include <semphr.h>
#include "../microphone/stm32l475e_iot01_audio.h"
#include "../microphone/microphone.h"

/* -------------------------- INIT DATA FOR MICROPHONE -------------------------- */

static BSP_AUDIO_Init_t MicParams;
    

static int16_t *BUFFER_RIS = (int16_t*)calloc(AUDIO_SAMPLING_FREQUENCY * 2, sizeof(int16_t));

int mic_init(){
    
    if (!BUFFER_RIS) {
            printf("Failed to allocate BUFFER_RIS buffer\n");
            return -1;
        }
    
    init_buffer(BUFFER_RIS);

    // set up the microphone
    MicParams.BitsPerSample = 16;
    MicParams.ChannelsNbr = AUDIO_CHANNELS;
    MicParams.Device = AUDIO_IN_DIGITAL_MIC1;
    MicParams.SampleRate = AUDIO_SAMPLING_FREQUENCY;
    MicParams.Volume = 32;

    int32_t ret = BSP_AUDIO_IN_Init(AUDIO_INSTANCE, &MicParams);

    if (ret != BSP_ERROR_NONE) {
        printf("Error Audio Init (%ld)\r\n", ret);
        return -1;
    } else {
        printf("OK Audio Init\t(Audio Freq=%ld)\r\n", AUDIO_SAMPLING_FREQUENCY);
    }

}

void taskTakeMic(void *pvParameters){
  while(true){
    start_recording();
  }
}

void setup() {
  if(mic_init() == -1){
    printf("Something goes wrong");
    return;
  }
  xTaskCreate(taskTakeMic, "taskMic", 115, NULL, 0, NULL);

  vTaskStartScheduler();                                                /* explicit call needed */
  Serial.println("Insufficient RAM");
}

void loop() {
  // put your main code here, to run repeatedly:

}
