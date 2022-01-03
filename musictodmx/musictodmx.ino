#include <STM32FreeRTOS.h>
#include <timers.h>
#include <semphr.h>
#include <stdlib.h>
#include <string.h>
#include "arduinoFFT.h"
#include "manage_output.h"
#include "beat_detection.h"
#include "fog.h"
#include "fire.h"

#define SAMPLES 2048
#define FRAME_LENGTH 100
#define MS_IN_MIN 60000
#define THRESHOLD_MEAN 200    //TODO: need to be checked if it is good enough

SemaphoreHandle_t buffer_mtx;
SemaphoreHandle_t new_data_mtx;
SemaphoreHandle_t color_mtx;
SemaphoreHandle_t mov_mtx;

float *buffer;

float lag = 100; //number of elements that it's used to model data
float influence = 0.75; //how signal influence average and deviation
float threshold = 2; //how much the singal must be different from deviation for activate

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

    TickType_t xLastWakeTime;
    unsigned long startTime = 0;
    unsigned long finishTime = 0;
    // xFreq is set to 1/4 of seconds but need to be set after timer analysis of processing and output
    TickType_t xFreq = 250 / (portTICK_PERIOD_MS);


    xLastWakeTime = xTaskGetTickCount();
    while(true){
        xSemaphoreTake(buffer_mtx, portMAX_DELAY);
        long currentAverage = 0;
        long currentMaximum = 0;
        long currentMinimum = MAXIMUM_SIGNAL_VALUE;
        
        for (int i = 0; i < FHT_N; i++) { // save 256 samples
          
          int k = analogRead(A0);
          
          currentMinimum = min(currentMinimum, (long)k);
          currentMaximum = max(currentMaximum, (long)k);
          currentAverage += k;
          
          k -= 0x0200; // form into a signed int
          k <<= 6; // form into a 16b signed int
          k <<= FreqGainFactorBits;
          
          fht_input[i] = k; // put real data into bins
        }
  
        currentAverage /= FHT_N;
        
        int signalDelta = currentMaximum - currentAverage;
        currentSignal = currentAverage + (2 * signalDelta);
        
        constrain(currentSignal, 0, currentMaximum);
        
        processHistoryValues(
          signals, 
          frequencyMagnitudeSampleIndex, 
          currentSignal, 
          totalSignal, 
          averageSignal, 
          signalVariance
        );
        //Serial.println("BUFFER FULL");
        c=0;
        if(uxSemaphoreGetCount(new_data_mtx) == 0){
            xSemaphoreGive(new_data_mtx);
        }
        //Serial.println("readIMU end task " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputRec"))));
                
        //Serial.println((String) "Task RecordingInput elapsed: "+ finishTime + " ms");
        xSemaphoreGive(buffer_mtx);
        vTaskDelayUntil(&xLastWakeTime, xFreq);    
        
    }
}

void taskInputProcessing(void *pvParameters){
  unsigned long startTime = 0;
  unsigned long finishTime = 0;
  
  while(true){
    xSemaphoreTake(new_data_mtx, portMAX_DELAY);
    
    startTime = micros();

    xSemaphoreTake(buffer_mtx, portMAX_DELAY);
    
    getFrequencyData();
    processFrequencyData();
    if(updateBeatProbability() == 1){
        xSemaphoreGive(color_mtx);
    }
    
    xSemaphoreGive(buffer_mtx);

    finishTime = (micros() - startTime)/1000;
    //Serial.println((String) "Task ProcessingInput elapsed: "+ finishTime + " ms");
    //Serial.println((String)"Counter peaks for average: " + n + "in "+finish+" s");
    
  }
}
// TODO check if the refresh frequency is correct, if send packet with 512 ch--> 44Hz, 23ms to send a packet
void taskSendingOutput(void *pvParameters){
    
    TickType_t xLastWakeTime;
    TickType_t xFreq;


    xLastWakeTime = xTaskGetTickCount();
    uint8_t light_mode, mov_mode;
    while(true){        
       
        xFreq = MS_IN_MIN / (300*portTICK_PERIOD_MS);      
        //Serial.println((String)"bpm = " + bpm);
        //Serial.println((String)"xFreq = " + xFreq  +" time elapsed= "+finishTime);
        
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
        
        send_output(bpm, light_mode, mov_mode);
        vTaskDelayUntil(&xLastWakeTime, xFreq);
    
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


    Serial.print("inputRec " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputRec"))));


    Serial.print("inputProc " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputProc"))));


    unsigned long startTime = 0;
    unsigned long finishTime = 0;
    unsigned long maxTime = 0;

    startTime = micros();
    // Insert code to test here
    finishTime = micros();
    maxTime = max(finishTime - startTime, maxTime);
 
    Serial.println((String)"MaxTime: " + (maxTime/1000000)+ " s");
    
    Serial.print("    ");
    Serial.println((String)"Time elapsed" + ((finishTime - startTime)/1000000)+" s" );
}

void setup(){
    setupADC();
    for (int i = 0; i < FREQUENCY_MAGNITUDE_SAMPLES; i++) {
      overallFrequencyMagnitudes[i] = 0;
      firstFrequencyMagnitudes[i] = 0;
      secondFrequencyMagnitudes[i] = 0;
      signals[i] = 0;
    }
  
    Serial.begin(115200);
    fireSelector();
    fogSelector();
    init_fixture();
    buffer = (float*)calloc(SAMPLES,sizeof(float));
    
    buffer_mtx = xSemaphoreCreateMutex();                               /* semaphores for buffer*/
    new_data_mtx = xSemaphoreCreateBinary();
    mov_mtx = xSemaphoreCreateBinary();
    color_mtx = xSemaphoreCreateBinary();

    xSemaphoreGive(buffer_mtx);
    
    xTaskCreate(taskInputRecording, "inputRec", 200/*160*/, NULL, 1, NULL); 
    //this task must have higher priority than inputRec bc otherwise it doesn't run
    xTaskCreate(taskInputProcessing, "inputProc", 16000, NULL,1 , NULL);
    //ELABORATION TASK 
    xTaskCreate(taskSendingOutput, "outputSend", 200, NULL, 3, NULL); 
    //TimerHandle_t xTimer = xTimerCreate("Valuate", pdMS_TO_TICKS(FRAME_LENGTH), pdTRUE, 3, taskValuate);
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

   
