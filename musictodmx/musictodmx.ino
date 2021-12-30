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

#define SAMPLES 2048
#define FRAME_LENGTH 100
#define MS_IN_MIN 60000
#define THRESHOLD_MEAN 200    //TODO: need to be checked if it is good enough

SemaphoreHandle_t buffer_mtx;
SemaphoreHandle_t new_data_mtx;
SemaphoreHandle_t bpm_mtx;
SemaphoreHandle_t color_mtx;
SemaphoreHandle_t mov_mtx;

LowPassFilter *filter;
double *buffer;
double max_peak = 0, mean_peak = 0, mean_peak_prev = 0, imp_sum = 0;
int n = 0, counter_peaks_buffer = 0;

float imp[10] = {1,0.9,0.8,0.7,0.7,0.5,0.4,0.4,0.4,0.3};
list *peak_arr;

arduinoFFT FFT = arduinoFFT();
unsigned long start;

HardwareSerial Serial4(D0, D1);
RS485Class RS485(Serial4, RS485_DEFAULT_TX_PIN, RS485_DEFAULT_DE_PIN, RS485_DEFAULT_RE_PIN);

//try to set a default value
int16_t bpm = 120;

/*-------------------- PERIODIC TASK ------------------------*/

// TODO check if the sample frequency is correct
void taskInputRecording(void *pvParameters){
    //Serial.println("readIMU "+ String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputRec"))));
    uint16_t c = 0;
    int filtered;

    TickType_t xLastWakeTime;
    unsigned long startTime = 0;
    unsigned long finishTime = 0;
    // xFreq is set to 1/4 of seconds but need to be set after timer analysis of processing and output
    TickType_t xFreq = 250 / (portTICK_PERIOD_MS);


    xLastWakeTime = xTaskGetTickCount();
    while(true){
        xSemaphoreTake(buffer_mtx, portMAX_DELAY);
        startTime = micros();
        while(c < SAMPLES){
            buffer[c] = (double)analogRead(A0);
            //Serial.println((String)"Read n." + c + ": "+buffer[c] );
            c++;
        }
        Serial.println("BUFFER FULL");
        c=0;
        if(uxSemaphoreGetCount(new_data_mtx) == 0){
            xSemaphoreGive(new_data_mtx);
        }
        //Serial.println("readIMU end task " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputRec"))));
        
        finishTime = (micros() - startTime)/1000;
        
        //Serial.println((String) "Task RecordingInput elapsed: "+ finishTime + " ms");
        xSemaphoreGive(buffer_mtx);
        vTaskDelayUntil(&xLastWakeTime, xFreq);    
        
    }
}

void taskInputProcessing(void *pvParameters){
  //double buffer_im[SAMPLES];
  unsigned long startTime = 0;
  unsigned long finishTime = 0;
  
  double filtered, peak_fil, max_peak_fil, min_peak_fil;

  while(true){
    Serial.println("processing");
    xSemaphoreTake(new_data_mtx, portMAX_DELAY);
    
    startTime = millis();
    //memset(buffer_im, 0, SAMPLES*sizeof(double));

    xSemaphoreTake(buffer_mtx, portMAX_DELAY);
    peak_fil = 0;
    for(int i = 0; i < SAMPLES; i++){
      LowPassFilter_put(filter,buffer[i]);
      filtered = LowPassFilter_get(filter);
      peak_fil = max(peak_fil, filtered);
    }
    max_peak_fil = max(max_peak_fil,peak_fil);
    min_peak_fil = min(min_peak_fil, peak_fil);
    
    /*FFT.Windowing(buffer,SAMPLES,FFT_WIN_TYP_HAMMING,FFT_FORWARD);

    FFT.Compute(buffer,buffer_im,SAMPLES, FFT_FORWARD);

    FFT.ComplexToMagnitude(buffer,buffer_im,SAMPLES);
    //TODO: need to fine tune the third param
    //TODO: need to focus only on bass peak (freq 50Hz - 200Hz)
    double peak = FFT.MajorPeak(buffer,SAMPLES,9600);

    Serial.println((String) "Comparison peaks: peak --> "+peak+" peak_fil --> "+peak_fil);
    */
    peak_arr = add_first(peak_arr,peak_fil);
    Serial.print("add an element to list  ");
    Serial.println(n);
    n++;

    /* Each 1000 iterations, reset the minimum and maximum detected values.
     This helps if the sound level changes and we want our code to adapt to it.*/

    if((n%1000) == 0){
      max_peak_fil = 0;
      min_peak_fil = 1023;
    }

    mean_peak_prev = mean_peak;
    if(n>10){
      delete_last(peak_arr);
      mean_peak = 0;
      list *l = peak_arr; 
      for(int i = 0;i<10; i++){
        mean_peak += imp[i]*l->value;
        l = l->next; 
      }
      mean_peak = mean_peak/imp_sum; //media pesata
      //everytime it detect a peak it let lights change 
      /*for(int i = 0; i < SAMPLES; i++){
        if(buffer[i] >= mean_peak){ //only one time for rec maybe, maybe useful if use only peak
          counter_peaks_buffer++;
          xSemaphoreGive(color_mtx);
        }
      }*/
      if(peak_fil >= mean_peak-30){
        xSemaphoreGive(color_mtx);
      }
    //Serial.println((String)"prev peak = " + mean_peak_prev + "  mean peak now = " + mean_peak);
    //check if is changed the rhythm of the song to change mov 
    //control if with average or with peack value
    if(mean_peak - mean_peak_prev >= THRESHOLD_MEAN ){
      xSemaphoreGive(mov_mtx);
    }
    }
    xSemaphoreGive(buffer_mtx);
    xSemaphoreTake(bpm_mtx,portMAX_DELAY);
    //finishTime = (millis() - startTime)/1000;
    finishTime = millis();
    bpm = counter_peaks_buffer * 60/finishTime;
    float freq = ((float)SAMPLES * (float)1000) / ((float)finishTime - (float)startTime);
    Serial.println((String)"FREQ  " + freq);
    //Serial.println((String) "Task ProcessingInput elapsed: "+ finishTime + " ms");
    xSemaphoreGive(bpm_mtx);
    
    Serial.println((String)"Peak value: " + peak_fil);
  
    //Serial.println((String)"Counter peaks for average: " + n + "in "+finish+" s");
    
    Serial.println((String)"Mean peak value: " + mean_peak);
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
    unsigned long startTime = 0;
    unsigned long finishTime = 0;
    TickType_t xFreq;


    xLastWakeTime = xTaskGetTickCount();
    uint8_t light_mode, mov_mode;
    while(true){
        xSemaphoreTake(bpm_mtx,portMAX_DELAY);
        
        startTime = micros();
        xFreq = MS_IN_MIN / (300*portTICK_PERIOD_MS);
        
        //we don't use bpm anymore
        xSemaphoreGive(bpm_mtx);
        
        //Serial.println((String)"bpm = " + bpm);
        //Serial.println((String)"xFreq = " + xFreq  +" time elapsed= "+finishTime);
        
        if(uxSemaphoreGetCount(color_mtx) > 0){
          xSemaphoreTake(color_mtx,portMAX_DELAY);
          Serial.println((String)"COLOR SEM " + uxSemaphoreGetCount(color_mtx));
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
        
        send_output(bpm, light_mode, mov_mode);

        Serial.println(bpm);
        finishTime = (micros() - startTime) / 1000;
        Serial.println((String) "Task sendOutput time elapse: " + finishTime + " ms");
        
        vTaskDelayUntil(&xLastWakeTime, xFreq);

        Serial.println("SVEGLIA]");

        
    }
}

/*-------------------- ASYNC TASK ------------------------*/
void taskFog(void *pvParameters) {
    /* Block for DURATION. */
  TickType_t xLastWakeTimeFog;
  const TickType_t xFreqFog = FOG_DURATION_TIME / portTICK_PERIOD_MS;
  unsigned long startTime = 0;
  unsigned long finishTime = 0;

  xLastWakeTimeFog = xTaskGetTickCount();
  while (true) {
    vTaskSuspend(NULL);                                                 /* suspends itself */
    startTime = micros();
    fogStart();
    Serial.println("[Fog button pressed !]");
    vTaskDelayUntil(&xLastWakeTimeFog, xFreqFog);
    finishTime = (micros() - startTime)/1000;
    Serial.println((String) "FOG Freq = " + + " Fog time elapsed = "+ finishTime);
    fogStop();

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
    for(int i = 0 ; i<10; i++){
      imp_sum += imp[i];
    }

    Serial.begin(115200);
    fireSelector();
    fogSelector();
    init_fixture();
    buffer = (double*)calloc(SAMPLES,sizeof(double));
    filter = new LowPassFilter();
    LowPassFilter_init(filter);

    buffer_mtx = xSemaphoreCreateMutex();                               /* semaphores for buffer*/
    new_data_mtx = xSemaphoreCreateBinary();
    bpm_mtx = xSemaphoreCreateMutex();
    mov_mtx = xSemaphoreCreateBinary();
    color_mtx = xSemaphoreCreateBinary();

    xSemaphoreGive(buffer_mtx);
    xSemaphoreGive(bpm_mtx); 

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
