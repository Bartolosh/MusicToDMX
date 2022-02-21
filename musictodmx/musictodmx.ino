#include <STM32FreeRTOS.h>
#include <timers.h>
#include <semphr.h>
#include <stdlib.h>
#include <string.h>

#include "manage_output.h"
#include "list_custom.h"
#include "fog.h"
#include "fire.h"
#include "LowPassFilter.h"
#include "LowCutFilter.h"

#define SAMPLES 170
#define FRAME_LENGTH 100
#define MS_IN_MIN 60000
#define THRESHOLD_MOV 400   
#define THRESHOLD 350 

SemaphoreHandle_t buffer_mtx;
SemaphoreHandle_t new_data_mtx;
SemaphoreHandle_t bpm_mtx;
SemaphoreHandle_t color_mtx;
SemaphoreHandle_t mov_mtx;
SemaphoreHandle_t fog_mtx;
SemaphoreHandle_t fire_mtx;

LowCutFilter *cutfilter;
LowPassFilter *filter;
int buffer[SAMPLES];
int max_peak = 0, mean_peak = 0, mean_peak_prev = 0, imp_sum = 0;
int min_peak = 0;
int n = 0, counter_peaks_buffer = 0;

unsigned long startTime = 0;
unsigned long finishTime = 0;


HardwareSerial Serial4(D0, D1);
RS485Class RS485(Serial4, RS485_DEFAULT_TX_PIN, RS485_DEFAULT_DE_PIN, RS485_DEFAULT_RE_PIN);

uint16_t bpm = 120;

/*-------------------- PERIODIC TASK ------------------------*/

void taskInputRecording(void *pvParameters){
    uint16_t c = 0;
    int filtered;

    TickType_t xLastWakeTime;
    TickType_t xNextRead;
    /* xFreq is set to 1/4  of seconds but need to be set after ti100mer analysis of processing and output */
    const TickType_t xFreq = 250 / (portTICK_PERIOD_MS);
    const TickType_t xFreqRec = 1 / (portTICK_PERIOD_MS);

    xLastWakeTime= xTaskGetTickCount();
    xNextRead = xTaskGetTickCount();
    while(true){
        
        xSemaphoreTake(buffer_mtx, portMAX_DELAY);
        
        
        while(c < SAMPLES){
            
            
            buffer[c] = (int)analogRead(A0);
            c++;
            
            vTaskDelayUntil(&xNextRead, xFreqRec);
            
        }
        
        c=0;
        if(uxSemaphoreGetCount(new_data_mtx) == 0){
            xSemaphoreGive(new_data_mtx);
        }
 
        xSemaphoreGive(buffer_mtx);
        
        vTaskDelayUntil(&xLastWakeTime, xFreq);
        
    }
}

void taskInputProcessing(void *pvParameters){


  unsigned long lastChange = 0;
  unsigned long thisChange = 0;
        
  int cutted,filtered, peak_fil, max_peak_fil, min_peak_fil;
  int last_peak = 0;
  long lvl_sound = 0, peak_to_peak = 0;

  max_peak_fil = 0;
  min_peak_fil = 1023;

  while(true){
    
    xSemaphoreTake(new_data_mtx, portMAX_DELAY);
    xSemaphoreTake(buffer_mtx, portMAX_DELAY);
    startTime = millis();
    peak_fil = 0;

    for(int i = 0; i < SAMPLES; i++){
      /* cutting low freq (0 Hz -20 Hz) */
      LowCutFilter_put(cutfilter,buffer[i]);
      cutted = LowCutFilter_get(cutfilter);
      
      /* filtering signal */
      LowPassFilter_put(filter,cutted);
      filtered = LowPassFilter_get(filter);
      filtered = abs(filtered);
      
      peak_fil = max(peak_fil, filtered);
      
    }
    xSemaphoreGive(buffer_mtx);
    max_peak_fil = max(max_peak_fil,peak_fil);
    min_peak_fil = min(min_peak_fil, peak_fil);

    n++;
        
    /* Each 1000 iterations,reset the minimum and maximum detected values.
     This helps if the sound level changes and we want our code to adapt to it.*/
    if((n%100) == 0){
      lvl_sound = 0;
    }

    if((n%80) == 0){
      
      max_peak_fil = 0;
      min_peak_fil = 1023;
    }

    int lvl = map(peak_fil, min_peak_fil, max_peak_fil, 0, 1023);
    Serial.println(lvl);
    
    if(lvl > THRESHOLD){
      if((millis()-lastChange)>300){
        xSemaphoreGive(color_mtx);
        thisChange = millis();
        peak_to_peak = thisChange-lastChange;
        lastChange = thisChange;
        xSemaphoreTake(bpm_mtx,portMAX_DELAY);
        //Serial.println(bpm);
        bpm = 60000/peak_to_peak;
  
        xSemaphoreGive(bpm_mtx);
      }
    }
    if(peak_to_peak > (lvl_sound - THRESHOLD_MOV)){
      lvl_sound = peak_to_peak;
      peak_to_peak = 0;
      xSemaphoreGive(mov_mtx);
    }   
      
    finishTime = (millis() - startTime);

    Serial.println((String) "Task Rec + Proc elapsed: "+ finishTime + " ms");
   
    //Serial.println((String) "Task ProcessingInput elapsed: "+ (finish_time- start_time) + " ms");
        
  }
}

void taskSendingOutput(void *pvParameters){
    
    TickType_t xLastWakeTime_out;
    
    const TickType_t xFreq_out = 41 / portTICK_PERIOD_MS;;

    xLastWakeTime_out = xTaskGetTickCount();
    uint8_t light_mode, mov_mode;
    uint8_t fog_state = STOP;
    uint8_t fire_state = STOP;
    int duration = 0, duration_end = 0;

    while(true){
      
        
        if(uxSemaphoreGetCount(color_mtx) > 0){
          
          xSemaphoreTake(color_mtx,portMAX_DELAY);
          
          light_mode = 1;
        }
        else{    
          light_mode = 0;
        }
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
          if((duration - duration_end) >= 1500){
            duration = 0; 
            duration_end = 0;
            fog_state = STOP;
            xSemaphoreGive(fog_mtx);   
          }
        }
        if(uxSemaphoreGetCount(fire_mtx) == 0){
          xSemaphoreGive(fog_mtx);
          fire_state=1;
        }
        else{
          fire_state=0;
        }
        send_output(bpm, light_mode, mov_mode, fog_state, fire_state);
        /*finishTime = millis() - startTime;*/
        
        /*Serial.println((String) "Task sendOutput time elapse: " + finishTime + " ms");*/

        //vTaskDelayUntil(&xLastWakeTime_out,xFreq_out); 
    }
}

/*-------------------- ASYNC TASK ------------------------*/
void taskFog(void *pvParameters) {
    /* Block for DURATION. */
  TickType_t xLastWakeTimeFog;
  TickType_t xFreqFog = FOG_DURATION_TIME / portTICK_PERIOD_MS;
  xLastWakeTimeFog = xTaskGetTickCount();
  while (true) {
    vTaskSuspend(NULL);                                                 /* suspends itself */
    
    vTaskDelayUntil(&xLastWakeTimeFog, xFreqFog);
    if(uxSemaphoreGetCount(fog_mtx) == 1){
      xSemaphoreTake(fog_mtx, portMAX_DELAY);
    }
   }
}


void taskFire(void *pvParameters) {
  while (true) {
    vTaskSuspend(NULL);                                                 /* suspends itself */

    if(uxSemaphoreGetCount(fire_mtx) == 1){
      xSemaphoreTake(fire_mtx, portMAX_DELAY);
    }
    
    //Serial.println("FIRE TASK =  " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("fireStart"))));
  }
}

/*-------------------- VALUATING TASK --------------------*/

void taskValuate(TimerHandle_t xTimer){
  
    // Serial.print("inputProc " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputProc"))));


    unsigned long startTime = 0;
    unsigned long finishTime = 0;
    unsigned long maxTime = 0;
    double filtered, peak;

    startTime = millis();
    /* Insert code to test here */
    for(int k=0; k<SAMPLES; k++){
      buffer[k] = (double) analogRead(A0);    buffer;
      //LowPassFilter_put(filter, buffer[k]);xSemaphoreCreateBinary()
      //filtered = LowPassFilter_get(filter);

      //peak = max(peak, filtered);
    }
    finishTime = millis();
    maxTime = max(finishTime - startTime, maxTime);
    float freq = ((float)SAMPLES * (float)1000) / ((float)finishTime - (float)startTime);
    Serial.println((String)"MaxTime: " + maxTime + " ms");
    
    Serial.print("    ");
    Serial.println((String)"Time elapsed" + ((finishTime - startTime)/1000000)+" s" );
}

void setup(){
   
 while (1) { 
    Serial.begin(115200);
    fireSelector();
    fogSelector();
    init_fixture();
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
    fire_mtx = xSemaphoreCreateBinary(); 

    xSemaphoreGive(buffer_mtx);
    xSemaphoreGive(bpm_mtx);
    xSemaphoreGive(fog_mtx);
    xSemaphoreGive(fire_mtx);

    xTaskCreate(taskInputRecording, "inputRec", 200/*95*/, NULL, 2, NULL); 
    /* this task must have higher priority than inputRec bc otherwise it doesn't run */
    xTaskCreate(taskInputProcessing, "inputProc", 90/*70*/, NULL,2 , NULL);
    /* ELABORATION TASK */
    xTaskCreate(taskSendingOutput, "outputSend", 72/*68*/, NULL, 1, NULL); 
    //TimerHandle_t xTimer = xTimerCreate("Valuate", pdMS_TO_TICKS(FRAME_LENGTH), pdTRUE, (void*)3, taskValuate);
    //xTimerStart(xTimer, 0);   
    
    xTaskCreate(taskFog, "fogStart", 62, NULL, 3, &taskFogHandle);
    xTaskCreate(taskFire, "fireStart", 62, NULL, 3, &taskFireHandle);
    
    vTaskStartScheduler();                                                /* explicit call needed */
    Serial.println("Insufficient RAM");
}
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
