#include <STM32FreeRTOS.h>
#include <timers.h>
#include <semphr.h>
#include <stdlib.h>

#include "../manage_output.h"


#define SAMPLES 2048

SemaphoreHandle_t mtxToBuffer;

float *buffer;

// TODO check if the sample frequency is correct
void taskInputRecording(void *pvParameters){
    while(true){
        xSemaphoreTake(mtxToBuffer, portMAX_DELAY);
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
        xSemaphoreGive(mtxToBuffer);
        
    }
}

// TODO check if the sample frequency is correct
void taskSendingOutput(void *pvParameters){
    while(true){
        int bpm = (int)pvParameters; //TODO control if out while, and if dmx work
        send_output(bpm);
    }
}


void setup(){
    Serial.begin(115200);
    buffer = (float*)calloc(SAMPLES,sizeof(float));
    mtxToBuffer = xSemaphoreCreateBinary();                               /* semaphores for buffer*/
    xSemaphoreGive(mtxToBuffer);

    xTaskCreate(taskInputRecording, "inputRec", 115, NULL, 0, NULL); 

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
