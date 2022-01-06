#include <STM32FreeRTOS.h>
#include <timers.h>
#include <semphr.h>
#include <stdlib.h>
#include <string.h>
#include "arduinoFFT.h"
#include "manage_output.h"
#include "list_custom.h"
#include "fog.h"
#include "fire.h"
#include "LowPassFilter.h"
#include "LowCutFilter.h"

#define SAMPLES 2048
#define FRAME_LENGTH 100
#define MS_IN_MIN 60000
#define THRESHOLD_MOV 400    //TODO: need to be checked if it is good enough
#define THRESHOLD 250 //TUNE IT define a peak

SemaphoreHandle_t buffer_mtx;
SemaphoreHandle_t new_data_mtx;
SemaphoreHandle_t bpm_mtx;
SemaphoreHandle_t color_mtx;
SemaphoreHandle_t mov_mtx;
SemaphoreHandle_t fog_mtx;

LowCutFilter *cutfilter;
LowPassFilter *filter;
int *buffer;
int max_peak = 0, mean_peak = 0, mean_peak_prev = 0, imp_sum = 0;
int min_peak = 0;
int n = 0, counter_peaks_buffer = 0;

unsigned long startTime = 0;
unsigned long finishTime = 0;


HardwareSerial Serial4(D0, D1);
RS485Class RS485(Serial4, RS485_DEFAULT_TX_PIN, RS485_DEFAULT_DE_PIN, RS485_DEFAULT_RE_PIN);

//try to set a default value
uint16_t bpm = 120;

/*-------------------- PERIODIC TASK ------------------------*/

// TODO check if the sample frequency is correct
void taskInputRecording(void *pvParameters){
    //Serial.println("readIMU "+ String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputRec"))));
    uint16_t c = 0;
    int filtered;

    TickType_t xLastWakeTime;
    // xFreq is set to 1/4anzi  of seconds but need to be set after timer analysis of processing and output
    TickType_t xFreq = 250 / (portTICK_PERIOD_MS);


    xLastWakeTime = xTaskGetTickCount();
    while(true){
        xSemaphoreTake(buffer_mtx, portMAX_DELAY);
        startTime = millis();
        while(c < SAMPLES){
            buffer[c] = (double)analogRead(A0);
            //Serial.println((String)"Read n." + c + ": "+buffer[c] );
            c++;
        }
        //Serial.println("BUFFER FULL");
        c=0;
        if(uxSemaphoreGetCount(new_data_mtx) == 0){
            xSemaphoreGive(new_data_mtx);
        }
        //Serial.println("readIMU end task " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputRec"))));
        
        finishTime = (millis() - startTime)/1000;
        
        //Serial.println((String) "Task RecordingInput elapsed: "+ finishTime + " ms");
        xSemaphoreGive(buffer_mtx);
        vTaskDelayUntil(&xLastWakeTime, xFreq);    
        
    }
}

void taskInputProcessing(void *pvParameters){
  
  unsigned long lastChange = 0;
  unsigned long thisChange = 0;
  int cutted,filtered, peak_fil, max_peak_fil, min_peak_fil;

  long lvl_sound = 0, peak_to_peak = 0;

  max_peak_fil = 0;
  min_peak_fil = 1023;
  uint8_t alterna = 0;

  while(true){
    xSemaphoreTake(new_data_mtx, portMAX_DELAY);

    xSemaphoreTake(buffer_mtx, portMAX_DELAY);
    peak_fil = 0;

    

    for(int i = 0; i < SAMPLES; i++){
      // cutting low freq (0 Hz -20 Hz)
      LowCutFilter_put(cutfilter,buffer[i]);
      cutted = LowCutFilter_get(cutfilter);
      // filtering signal
      LowPassFilter_put(filter,cutted);
      filtered = LowPassFilter_get(filter);
      peak_fil = max(peak_fil, filtered);
      
    }
    xSemaphoreGive(buffer_mtx);
    max_peak_fil = max(max_peak_fil,peak_fil);
    min_peak_fil = min(min_peak_fil, peak_fil);

    n++;

    /* Each 1000 iterations,reset the minimum and maximum detected values.
     This helps if the sound level changes and we want our code to adapt to it.*/
    if((n%400) == 0){
      lvl_sound = 0;
    }

    if((n%700) == 0){
      
      max_peak_fil = 0;
      min_peak_fil = 1023;
    }

    int lvl = map(peak_fil, min_peak_fil, max_peak_fil, 0, 1023);
    //Serial.println((String)"PEAK = " + lvl);
    
    if(lvl > THRESHOLD){
      xSemaphoreGive(color_mtx);
      thisChange = millis();
      peak_to_peak = thisChange-lastChange;
      lastChange = thisChange;
      xSemaphoreTake(bpm_mtx,portMAX_DELAY);
      
      bpm = 60000/peak_to_peak;

      xSemaphoreGive(bpm_mtx);
      
    }
    if(peak_to_peak > (lvl_sound - THRESHOLD_MOV)){
      lvl_sound = peak_to_peak;
      peak_to_peak = 0;
      //Serial.println("Change mov");
      xSemaphoreGive(mov_mtx);
    }
    //Serial.println((String)"sound: " + lvl_sound + " new = " + peak_to_peak);
    
    //finishTime = (millis() - startTime)/1000;
    finishTime = millis();
   
    float freq = ((float)SAMPLES * (float)1000) / ((float)finishTime - (float)startTime);
    Serial.println((String)"FREQ  " + freq);
    //Serial.println((String) "Task ProcessingInput elapsed: "+ finishTime + " ms");
    
    
    //Serial.println((String)"Peak value: " + peak_fil);

    /*Serial.println("Spectrum values:");
    Serial.print("[");
    for(int i = 0 ; i < SAMPLES; i++){
        Serial.print(buffer[i]);
        Serial.println(",");
    }
    Serial.println("]");*/
    
  }
}
// TODO check if the refresh frequency is correct, if send packet with 512 ch--> 44Hz, 23ms to send a packet
void taskSendingOutput(void *pvParameters){
    
    TickType_t xLastWakeTime;
    
    TickType_t xFreq;


    xLastWakeTime = xTaskGetTickCount();
    uint8_t light_mode, mov_mode;
    uint8_t fog_state = STOP;
    int duration = 0, duration_end = 0;

    while(true){
        xSemaphoreTake(bpm_mtx,portMAX_DELAY);
        
        xFreq = 23 / portTICK_PERIOD_MS;
        
        if(uxSemaphoreGetCount(color_mtx) > 0){
          
          xSemaphoreTake(color_mtx,portMAX_DELAY);
          
          light_mode = 1;
        }
        else{
          light_mode = 0;
        }
        xSemaphoreGive(bpm_mtx);
        if(uxSemaphoreGetCount(mov_mtx) > 0){
          xSemaphoreTake(mov_mtx,portMAX_DELAY);
          //TODO: add call for change mov speed
          mov_mode = 1;
        }
        else{
          mov_mode = 0;
        }

        if(uxSemaphoreGetCount(fog_mtx) == 0){
          duration = millis();
          if(duration_end == 0){
            duration_end = millis();
          }
          fog_state = START;
          Serial.println((duration - duration_end));
          if((duration - duration_end) >= 1500){
            duration = 0; 
            duration_end = 0;
            fog_state = STOP;
            xSemaphoreGive(fog_mtx);   
          }
        }
        
        send_output(bpm, light_mode, mov_mode, fog_state);

        //Serial.println((String) "Task sendOutput time elapse: " + finishTime + " ms");
        
        vTaskDelayUntil(&xLastWakeTime, xFreq);
    }
}

/*-------------------- ASYNC TASK ------------------------*/
void taskFog(void *pvParameters) {
    /* Block for DURATION. */
  TickType_t xLastWakeTimeFog;
  TickType_t xFreqFog = FOG_DURATION_TIME / portTICK_PERIOD_MS;
  unsigned long startTime = 0;
  unsigned long finishTime = 0;

  xLastWakeTimeFog = xTaskGetTickCount();
  while (true) {
    vTaskSuspend(NULL);                                                 /* suspends itself */
    startTime = micros();
    
    Serial.println("[Fog button pressed !]");
    vTaskDelayUntil(&xLastWakeTimeFog, xFreqFog);
    if(uxSemaphoreGetCount(fog_mtx) == 1){
      xSemaphoreTake(fog_mtx, portMAX_DELAY);
    }
    finishTime = (micros() - startTime)/1000;
    Serial.println((String) "FOG Freq = " + + " Fog time elapsed = "+ finishTime);

  }
}


void taskFire(void *pvParameters) {
  TickType_t xLastWakeTimeFire;
  const TickType_t xFreqFire = 500 / portTICK_PERIOD_MS;
  xLastWakeTimeFire = xTaskGetTickCount();
  while (true) {
    vTaskSuspend(NULL);                                                 /* suspends itself */
    
    Serial.println("[Fire button pressed !]");
    fireStart();
    vTaskDelayUntil(&xLastWakeTimeFire, xFreqFire); //maybe it isn't useful but task fog is dangerous
    fireStop();
    
  }
}

/*-------------------- VALUATING TASK --------------------*/

void taskValuate(TimerHandle_t xTimer){


    //Serial.print("inputRec " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputRec"))));


    // Serial.print("inputProc " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputProc"))));


    unsigned long startTime = 0;
    unsigned long finishTime = 0;
    unsigned long maxTime = 0;
    double filtered, peak;

    startTime = millis();
    // Insert code to test here
    for(int k=0; k<SAMPLES; k++){
      buffer[k] = (double) analogRead(A0);
      //LowPassFilter_put(filter, buffer[k]);
      //filtered = LowPassFilter_get(filter);

      //peak = max(peak, filtered);
    }
    finishTime = millis();
    maxTime = max(finishTime - startTime, maxTime);
    float freq = ((float)SAMPLES * (float)1000) / ((float)finishTime - (float)startTime);
    Serial.println((String)"MaxTime: " + maxTime + " ms");
    Serial.println((String)"Freq: " + freq +" peak: "+ peak);
    
    Serial.print("    ");
    Serial.println((String)"Time elapsed" + ((finishTime - startTime)/1000000)+" s" );
}

void setup(){
    
    Serial.begin(115200);
    fireSelector();
    fogSelector();
    init_fixture();
    buffer = (int*)calloc(SAMPLES,sizeof(int));
    cutfilter = new LowCutFilter();
    filter = new LowPassFilter();
    LowCutFilter_init(cutfilter);
    LowPassFilter_init(filter);

    buffer_mtx = xSemaphoreCreateMutex();                               /* semaphores for buffer*/
    new_data_mtx = xSemaphoreCreateBinary();
    bpm_mtx = xSemaphoreCreateMutex();
    fog_mtx = xSemaphoreCreateBinary();
    mov_mtx = xSemaphoreCreateBinary();
    color_mtx = xSemaphoreCreateBinary();

    xSemaphoreGive(buffer_mtx);
    xSemaphoreGive(bpm_mtx);
    xSemaphoreGive(fog_mtx);

    xTaskCreate(taskInputRecording, "inputRec", 200/*160*/, NULL, 1, NULL); 
    //this task must have higher priority than inputRec bc otherwise it doesn't run
    xTaskCreate(taskInputProcessing, "inputProc", 16000, NULL,1 , NULL);
    //ELABORATION TASK 
    xTaskCreate(taskSendingOutput, "outputSend", 200, NULL, 3, NULL); 
    //TimerHandle_t xTimer = xTimerCreate("Valuate", pdMS_TO_TICKS(FRAME_LENGTH), pdTRUE, (void*)3, taskValuate);
    //xTimerStart(xTimer, 0);   
    
    xTaskCreate(taskFog, "fogStart", 80, NULL, 4, &taskFogHandle);
    xTaskCreate(taskFire, "fireStart", 80, NULL, 4, &taskFireHandle);
    
    vTaskStartScheduler();                                                /* explicit call needed */
    Serial.println("Insufficient RAM");
}

void loop(){
    //ERROR IF WE ARE HERE
    /*
    TimerHandle_t xTimer = xTimerCreate("PPMGenerator", pdMS_TO_TICKS(FRAME_LENGTH), pdTRUE, 0, taskPPMGenerator);
    xTimerStart(xTimer, 0);  

    semReadFlex = xSemaphoreCreateBinary();                              
    xSemaphoreGive(semReadFlex);

    semDataProcess = xSemaphoreCreateCounting(2, 0);

    if (xSemaphoreTake(mtxLEDState, (TickType_t)5) == pdTRUE) {}

    xTaskCreate(taskReadIMU, "readIMU", 115, NULL, 0, NULL); 

    xTaskCreate(pointer to entry function, name, word allocated stack, void* parameters, priority, handle for task created)

    xSemaphoreTake(semReadFlex, portMAX_DELAY);

    xSemaphoreGive(semDataProcess);*/
}
