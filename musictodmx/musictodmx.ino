#include <STM32FreeRTOS.h>
#include <timers.h>
#include <semphr.h>
#include <stdlib.h>
#include <string.h>

#include "manage_output.h"
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
/*int buffer[SAMPLES];*/
int* buffer;
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
    /* xFreq is set to 1/4  of seconds */
    const TickType_t xFreq = 250 / (portTICK_PERIOD_MS);
    const TickType_t xFreqRec = 1 / (portTICK_PERIOD_MS);

    xLastWakeTime= xTaskGetTickCount();
    
    while(true){
       
        xSemaphoreTake(buffer_mtx, portMAX_DELAY);
       
        xNextRead = xTaskGetTickCount();
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
    peak_fil = 0;

    for(int i = 0; i < SAMPLES; i++){
      
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

    /* each 60 iterations reset peak for lighting movement */
    if((n%60) == 0){
      lvl_sound = 0;
    }

    /* each 80 iterations reset the peak to adapt light  */
    if((n%80) == 0){
      max_peak_fil = 0;
      min_peak_fil = 1023;
    }

    int lvl = map(peak_fil, min_peak_fil, max_peak_fil, 0, 1023);
 
    
    if(lvl > THRESHOLD){
      if((millis()-lastChange)>300){
        xSemaphoreGive(color_mtx);
        thisChange = millis();
        peak_to_peak = thisChange-lastChange;
        lastChange = thisChange;
        xSemaphoreTake(bpm_mtx,portMAX_DELAY);
        bpm = 60000/peak_to_peak;
  
        xSemaphoreGive(bpm_mtx);
      }
    }
    if(peak_to_peak > (lvl_sound - THRESHOLD_MOV)){
      lvl_sound = peak_to_peak;
      peak_to_peak = 0;
      xSemaphoreGive(mov_mtx);
    }   
        
  }
}

void taskSendingOutput(void *pvParameters){
    
    TickType_t xLastWakeTime_out;
    
    const TickType_t xFreq_out = 41 / portTICK_PERIOD_MS;

    xLastWakeTime_out = xTaskGetTickCount();
    uint8_t light_mode, mov_mode;
    uint8_t fog_state = STOP;
    uint8_t fire_state = STOP;
    int duration = 0, duration_end = 0;
    int count_fire = 0;

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
          if(count_fire == 3){
            xSemaphoreGive(fire_mtx);
          }
          count_fire++;
          fire_state=START;
        }
        else{
          fire_state=STOP;
          count_fire=0;
        }
        send_output(bpm, light_mode, mov_mode, fog_state, fire_state);
        vTaskDelayUntil(&xLastWakeTime_out,xFreq_out);
        
    }
}

/*-------------------- ASYNC TASK ------------------------*/
void taskFog(void *pvParameters) {
 
  while (true) {
    vTaskSuspend(NULL);                                             
    startTime = micros();
    if(uxSemaphoreGetCount(fog_mtx) == 1){
      xSemaphoreTake(fog_mtx, portMAX_DELAY);
    }
   }
}


void taskFire(void *pvParameters) {
  while (true) {
    vTaskSuspend(NULL);                                    
    
    if(uxSemaphoreGetCount(fire_mtx) == 1){
      xSemaphoreTake(fire_mtx, portMAX_DELAY);
    }
    
  }
}


void taskVal(void *pvParameters) {
  TickType_t xLastWakeTime_val;

  xLastWakeTime_val = xTaskGetTickCount();
  while(true){
    Serial.println("memory =  " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("outputSend"))));
  
     vTaskDelayUntil(&xLastWakeTime_val,((TickType_t)400/portTICK_PERIOD_MS));
  }
}

void setup(){
   
    Serial.begin(115200);
    fireSelector();
    fogSelector();
    init_fixture();
    cutfilter = new LowCutFilter();
    filter = new LowPassFilter();
    LowCutFilter_init(cutfilter);
    LowPassFilter_init(filter);

    buffer = (int*)calloc(SAMPLES, sizeof(int));
    
    buffer_mtx = xSemaphoreCreateMutex();   /* semaphores for buffer*/
    xSemaphoreGive(buffer_mtx);
    
    bpm_mtx = xSemaphoreCreateMutex();
    xSemaphoreGive(bpm_mtx);
    
    fog_mtx = xSemaphoreCreateBinary();
    xSemaphoreGive(fog_mtx);
   
    fire_mtx = xSemaphoreCreateBinary();
    xSemaphoreGive(fire_mtx);

    new_data_mtx = xSemaphoreCreateBinary();
    
    mov_mtx = xSemaphoreCreateBinary();
    
    color_mtx = xSemaphoreCreateBinary(); 
    /*Serial.println("START");*/

    xTaskCreate(taskInputRecording, "inputRec", 79, NULL, 2, NULL); 

    xTaskCreate(taskInputProcessing, "inputProc", 49, NULL,2 , NULL);
    
    xTaskCreate(taskSendingOutput, "outputSend",63 , NULL, 1, NULL); 


    /*xTaskCreate(taskVal, "validatetask", 80, NULL, 1, NULL);*/
    
    xTaskCreate(taskFog, "fogStart", 27, NULL, 3, &taskFogHandle);
    xTaskCreate(taskFire, "fireStart", 27, NULL, 3, &taskFireHandle);
    
    vTaskStartScheduler();                                                /* explicit call needed */
    Serial.println("Insufficient RAM");

}

void loop(){}
