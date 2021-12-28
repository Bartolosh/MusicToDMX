#include <STM32FreeRTOS.h>
#include <timers.h>
#include <semphr.h>
#include <stdlib.h>
#include <string.h>
#include "arduinoFFT.h"
#include "manage_output.h"
#include "fog.h"
#include "fire.h"

#define SAMPLES 2048
#define FRAME_LENGTH 100
#define MS_IN_MIN 60000

SemaphoreHandle_t buffer_mtx;
SemaphoreHandle_t new_data_mtx;
SemaphoreHandle_t bpm_mtx;
double *buffer;
double max_peak = 0, mean_peak = 0;
int n = 0, counter_peaks_buffer = 0;

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

    TickType_t xLastWakeTime;
    unsigned long startTime = 0;
    unsigned long finishTime = 0;
    // xFreq is set to 1/4 of seconds but need to be set after timer analysis of processing and output
    TickType_t xFreq = 250 / (portTICK_PERIOD_MS);


    xLastWakeTime = xTaskGetTickCount();
    while(true){
        Serial.println("REC: [");
        xSemaphoreTake(buffer_mtx, portMAX_DELAY);
        Serial.println((String)"prendo il semaforo del buffer rec" + uxSemaphoreGetCount(buffer_mtx));
        
        while(c < SAMPLES){
            buffer[c] = (double)analogRead(A0);
            //Serial.println((String)"Read n." + c + ": "+buffer[c] );
            c++;
        }
        Serial.println("BUFFER FULL");
        c=0;
        if(uxSemaphoreGetCount(new_data_mtx) == 0){
            Serial.println("aumento semaforo dei dati nuovi");
            xSemaphoreGive(new_data_mtx);
        }
        //Serial.println("readIMU end task " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputRec"))));
        vTaskDelayUntil(&xLastWakeTime, xFreq);

    
        Serial.println("rilascio il semaforo del buffer rec");
        
        Serial.println("]");    
        xSemaphoreGive(buffer_mtx);
        
        
    }
}

void taskInputProcessing(void *pvParameters){
  double buffer_im[SAMPLES];
  unsigned long startTime = 0;
  unsigned long finishTime = 0;
  while(true){
    
    Serial.println("PROCESSING [");
    
    xSemaphoreTake(new_data_mtx, portMAX_DELAY);
    Serial.println("prendo il semaforo dei nuovi dati");
    
    startTime = micros();
    memset(buffer_im, 0, SAMPLES*sizeof(double));
    //Serial.println("PROCESSING TASK");
    xSemaphoreTake(buffer_mtx, portMAX_DELAY);

    Serial.println((String)"prendo il semaforo del buffer" + uxSemaphoreGetCount(buffer_mtx));
    
    FFT.Windowing(buffer,SAMPLES,FFT_WIN_TYP_HAMMING,FFT_FORWARD);

    FFT.Compute(buffer,buffer_im,SAMPLES, FFT_FORWARD);

    FFT.ComplexToMagnitude(buffer,buffer_im,SAMPLES);
    
    //TODO: need to fine tune the third param
    //TODO: need to focus only on bass peak (freq 50Hz - 200Hz)
    double peak = FFT.MajorPeak(buffer,SAMPLES,9600);
    /*if (peak > max_peak){
        max_peak = peak;
    }*/
    n++;
    mean_peak = mean_peak + (peak -  mean_peak)/n;
    if(n>10){
    for(int i = 0; i < SAMPLES; i++){
      if(buffer[i] > mean_peak){
        counter_peaks_buffer++;
        //Serial.println((String) "Peaks counter: "+ counter_peaks_buffer);
      }
    }
    }
    Serial.println("prendo il semaforo del bpm");
    xSemaphoreTake(bpm_mtx,portMAX_DELAY);
    finishTime = (micros() - startTime)/1000;
    bpm = counter_peaks_buffer * 60/finishTime;
    Serial.println((String) "Task ProcessingInput elapsed: "+ finishTime + " ms");
    //Serial.println((String) "Estimated bpm: " + bpm);
    Serial.println("rilascio il semaforo dei bpm");
    xSemaphoreGive(bpm_mtx);
    
    //Serial.println((String)"Peak value: " + peak);
  
    //Serial.println((String)"Counter peaks for average: " + n + "in "+finish+" s");
    
    //Serial.println((String)"Mean peak value: " + mean_peak);
    /*Serial.println("Spectrum values:");
    Serial.print("[");
    for(int i = 0 ; i < SAMPLES; i++){
        Serial.print(buffer[i]);
        Serial.println(",");
    }
    Serial.println("]");*/
    Serial.println("rilascio il semaforo del buffer]");
    xSemaphoreGive(buffer_mtx);
  }
}
// TODO check if the refresh frequency is correct, if send packet with 512 ch--> 44Hz, 23ms to send a packet
void taskSendingOutput(void *pvParameters){
    
    TickType_t xLastWakeTime;
    unsigned long startTime = 0;
    unsigned long finishTime = 0;
    TickType_t xFreq;


    xLastWakeTime = xTaskGetTickCount();
    while(true){
        Serial.println("OUTPUT [");
        xSemaphoreTake(bpm_mtx,portMAX_DELAY);
        
        Serial.println("prendo il semaforo dei bpm");
        
        startTime = micros();
        xFreq = MS_IN_MIN / (bpm*portTICK_PERIOD_MS);
        bpm = (int)pvParameters; //TODO control if out while, and if dmx work
        //Serial.println((String)"bpm = " + bpm);
        //Serial.println((String)"xFreq = " + xFreq  +" time elapsed= "+finishTime);
        send_output(bpm);
        xSemaphoreGive(bpm_mtx);
        
        Serial.println("rilascio il semaforo dei bpm");

        Serial.println("Mi blocco in attesa del prossimo periodo");
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
    Serial.begin(115200);
    fireSelector();
    fogSelector();
    init_fixture();
    buffer = (double*)calloc(SAMPLES,sizeof(double));
    buffer_mtx = xSemaphoreCreateMutex();                               /* semaphores for buffer*/
    xSemaphoreGive(buffer_mtx); //NOW ALL SEMAPHORE LOCK FOREVER CONTROL IF USEFULs
    new_data_mtx = xSemaphoreCreateBinary();
    bpm_mtx = xSemaphoreCreateMutex();
    xSemaphoreGive(bpm_mtx); 

    xTaskCreate(taskInputRecording, "inputRec", 250/*160*/, NULL, 1, NULL); 
    //this task must have higher priority than inputRec bc otherwise it doesn't run
    xTaskCreate(taskInputProcessing, "inputProc", 5000, NULL,2 , NULL);
    //ELABORATION TASK 
    xTaskCreate(taskSendingOutput, "outputSend", 250, (void *)bpm, 3, NULL); 
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
