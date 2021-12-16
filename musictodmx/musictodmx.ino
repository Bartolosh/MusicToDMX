#include <STM32FreeRTOS.h>
#include <timers.h>
#include <semphr.h>
#include <stdlib.h>
#include "arduinoFFT.h"

#include "get_bpm.h"

#define SAMPLES 2048

SemaphoreHandle_t buffer_mtx;

float *buffer;

// TODO check if the sample frequency is correct
void taskInputRecording(void *pvParameters){
    while(true){
        xSemaphoreTake(buffer_mtx, portMAX_DELAY);
        uint16_t c = 0;
        while(c < SAMPLES){
            buffer[c] = (float)analogRead(A0);
            Serial.print("READ  ");
            Serial.print(c);
            Serial.print(": ");
            Serial.println(buffer[c]);
            c++;
        }
        Serial.println("BUFFER FULL");
        xSemaphoreGive(buffer_mtx);
        
    }
}

void taskInputProcessing(void *pvParameters){
     xSemaphoreTake(buffer_mtx, portMAX_DELAY);
     float *buffer_im[SAMPLES];
    arduinoFFT FFT = arduinoFFT();
    FFT.Windowing(buffer,SAMPLES,FFT_WIN_TYP_HAMMING,FFT_FORWARD);
    FFT.Compute(buffer,buffer_im,FFT_FORWARD);
    FFT.ComplexToMagnitude(buffer,buffer_im,SAMPLES);
    
    //TODO: need to fine tune the third param
    double peak = FFT.MajorPeak(buffer,SAMPLES,5000);
    Serial.println('Spectrum values:');
    Serial.print('[');
    for(int i = 0 ; i < SAMPLES; i++){
        Serial.print(buffer[i]);
        Serial.print(',');
    }
    Serial.println(']');
    xSemaphoreGive(buffer_mtx);
}

void setup(){
    Serial.begin(115200);
    buffer = (float*)calloc(SAMPLES,sizeof(float));
    buffer_mtx = xSemaphoreCreateBinary();                               /* semaphores for buffer*/
    xSemaphoreGive(buffer_mtx);

    xTaskCreate(taskInputRecording, "inputRec", 115, NULL, 0, NULL); 
    xTaskCreate(taskInputProcessing, "inputProc", 115, NULL, 0, NULL);
    
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

    xSemaphoreTake(semReadFlex, portMAX_DELAY);

    xSemaphoreGive(semDataProcess);*/
}
