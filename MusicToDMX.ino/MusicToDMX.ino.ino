#include <STM32FreeRTOS.h>
#include <timers.h>
#include <semphr.h>


void mic_init(){
    static uint16_t PCM_Buffer[PCM_BUFFER_LEN / 2];
    static BSP_AUDIO_Init_t MicParams;
    
    // Place to store final audio (alloc on the heap), here two seconds...
    static size_t TARGET_AUDIO_BUFFER_NB_SAMPLES = AUDIO_SAMPLING_FREQUENCY * 2;
    static int16_t *TARGET_AUDIO_BUFFER = (int16_t*)calloc(TARGET_AUDIO_BUFFER_NB_SAMPLES, sizeof(int16_t));
    static size_t TARGET_AUDIO_BUFFER_IX = 0;
    
    // we skip the first 50 events (100 ms.) to not record the button click
    static size_t SKIP_FIRST_EVENTS = 50;
    static size_t half_transfer_events = 0;
    static size_t transfer_complete_events = 0;
    if (!TARGET_AUDIO_BUFFER) {
            printf("Failed to allocate TARGET_AUDIO_BUFFER buffer\n");
            return 0;
        }

    // set up the microphone
    MicParams.BitsPerSample = 16;
    MicParams.ChannelsNbr = AUDIO_CHANNELS;
    MicParams.Device = AUDIO_IN_DIGITAL_MIC1;
    MicParams.SampleRate = AUDIO_SAMPLING_FREQUENCY;
    MicParams.Volume = 32;

    int32_t ret = BSP_AUDIO_IN_Init(AUDIO_INSTANCE, &MicParams);

    if (ret != BSP_ERROR_NONE) {
        printf("Error Audio Init (%ld)\r\n", ret);
        return 1;
    } else {
        printf("OK Audio Init\t(Audio Freq=%ld)\r\n", AUDIO_SAMPLING_FREQUENCY);
    }

}

void setup() {
  // put your setup code here, to run once:
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
