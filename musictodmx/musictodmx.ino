#include <STM32FreeRTOS.h>
#include <timers.h>
#include <semphr.h>
#include <stdlib.h>
#include "arduinoFFT.h"

#include "manage_output.h"


#define SAMPLES 2048

SemaphoreHandle_t buffer_mtx;
SemaphoreHandle_t new_data_mtx;

double *buffer;
double max_peak = 0;
int counter = 0;
arduinoFFT FFT = arduinoFFT();

int16_t bpm;

// TODO check if the sample frequency is correct
void taskInputRecording(void *pvParameters){
    uint16_t c = 0;
    while(true){
        xSemaphoreTake(buffer_mtx, portMAX_DELAY);
        while(c < SAMPLES){
            buffer[c] = (double)analogRead(A0);
            Serial.print("READ  ");
            Serial.print(c);
            Serial.print(": ");
            Serial.println(buffer[c]);
            c++;
        }
        Serial.println("BUFFER FULL");
        if(uxSemaphoreGetCount(new_data_mtx) == 0){
            xSemaphoreGive(new_data_mtx);
        }
        xSemaphoreGive(buffer_mtx);
        
    }
}

void taskInputProcessing(void *pvParameters){
  double buffer_im[SAMPLES];
  while(true){
    xSemaphoreTake(new_data_mtx, portMAX_DELAY);
    Serial.println("PROCESSING TASK");
    xSemaphoreTake(buffer_mtx, portMAX_DELAY);
    Serial.println("HO PRESO IL SEMAFORO FRA");
    FFT.Windowing(buffer,SAMPLES,FFT_WIN_TYP_HAMMING,FFT_FORWARD);
    Serial.println("HO PRESO IL SEMAFORO FRA 3");

    FFT.Compute(buffer,buffer_im,SAMPLES, FFT_FORWARD);
    Serial.println("HO PRESO IL SEMAFORO FRA 4");

    FFT.ComplexToMagnitude(buffer,buffer_im,SAMPLES);
    Serial.println("HO PRESO IL SEMAFORO FRA 5");
    
    //TODO: need to fine tune the third param
    double peak = FFT.MajorPeak(buffer,SAMPLES,5000);
    if (peak > max_peak){
        max_peak = peak;
        counter = 0;
    }
    counter++;
    Serial.print("Peak value: ");
    Serial.println(max_peak);
    Serial.print("Counter peak: ");
    Serial.println(counter);
    Serial.println("Spectrum values:");
    Serial.print("[");
    for(int i = 0 ; i < SAMPLES; i++){
        Serial.print(buffer[i]);
        Serial.print(",");
    }
    Serial.println("]");
    xSemaphoreGive(buffer_mtx);
  }
}
// TODO check if the refresh frequency is correct, if send packet with 512 ch--> 44Hz, 23ms to send a packet
void taskSendingOutput(void *pvParameters){
    while(true){
        bpm = (int)pvParameters; //TODO control if out while, and if dmx work
        send_output(bpm);
    }
}

void taskValuate(TimerHandle_t xTimer){


    Serial.print("inputRec " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputRec"))));


    //Serial.print("inputProc " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("inputProc"))));


    unsigned long startTime = 0;
    unsigned long finishTime = 0;
    unsigned long maxTime = 0;
    startTime = micros();
    finishTime = micros();
    maxTime = max(finishTime - startTime, maxTime);
    Serial.print(maxTime);
    Serial.print("    ");
    Serial.println(finishTime - startTime);
}

void setup(){
    Serial.begin(115200);
    buffer = (double*)calloc(SAMPLES,sizeof(double));
    buffer_mtx = xSemaphoreCreateBinary();                               /* semaphores for buffer*/
    xSemaphoreGive(buffer_mtx); //NOW ALL SEMAPHORE LOCK FOREVER CONTROL IF USEFUL

    new_data_mtx = xSemaphoreCreateBinary();

    xTaskCreate(taskInputRecording, "inputRec", 8000, NULL, 0, NULL); 
    xTaskCreate(taskInputProcessing, "inputProc", 8000, NULL, 0, NULL);
    //ELABORATION TASK 
    //xTaskCreate(taskSendingOutput, "outputSend", 115, (void *)bpm, 0, NULL); 

    TimerHandle_t xTimer = xTimerCreate("Valuate", pdMS_TO_TICKS(FRAME_LENGTH), pdTRUE, 0, taskValuate);
    xTimerStart(xTimer, 0);   

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
