#include <STM32FreeRTOS.h>
#include <timers.h>
#include <semphr.h>
#include <stdlib.h>
#include "arduinoFFT.h"

#include "../manage_output.h"


#define SAMPLES 2048

SemaphoreHandle_t buffer_mtx;

double *buffer;
arduinoFFT FFT = arduinoFFT();

int16_t bpm;

// TODO check if the sample frequency is correct
void taskInputRecording(void *pvParameters){
    while(true){
        xSemaphoreTake(buffer_mtx, portMAX_DELAY);
        uint16_t c = 0;
        while(c < SAMPLES){
            buffer[c] = (double)analogRead(A0);
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
    double buffer_im[SAMPLES] = {};
    FFT.Windowing(buffer,SAMPLES,FFT_WIN_TYP_HAMMING,FFT_FORWARD);
    FFT.Compute(buffer,buffer_im,SAMPLES, FFT_FORWARD);
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
// TODO check if the refresh frequency is correct, if send packet with 512 ch--> 44Hz, 23ms to send a packet
void taskSendingOutput(void *pvParameters){
    while(true){
        bpm = (int)pvParameters; //TODO control if out while, and if dmx work
        send_output(bpm);
    }
}


void setup(){
    Serial.begin(115200);
    buffer = (double*)calloc(SAMPLES,sizeof(double));
    buffer_mtx = xSemaphoreCreateBinary();                               /* semaphores for buffer*/
    xSemaphoreGive(buffer_mtx);

    xTaskCreate(taskInputRecording, "inputRec", 115, NULL, 0, NULL); 
    xTaskCreate(taskInputProcessing, "inputProc", 115, NULL, 0, NULL);
    //ELABORATION TASK 
    xTaskCreate(taskSendingOutput, "outputSend", 115, (void *)bpm, 0, NULL); 

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
