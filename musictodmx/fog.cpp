#include "fog.h"

uint16_t fog_channel = 0;

void init_fog(uint16_t ch){
    fog_channel = ch;
}

TaskHandle_t taskFogHandle = NULL;

//function that manage interrupt
void fogHandler(void) {
  //this function awake the task 
  BaseType_t xYieldRequired = xTaskResumeFromISR(taskFogHandle);   /* INCLUDE_vTaskSuspend and INCLUDE_xTaskResumeFromISR must be defined as 1 */
  portYIELD_FROM_ISR(xYieldRequired);
}

void fogSelector() {
  pinMode(FOG_BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(FOG_BUTTON_PIN), fogHandler, FALLING);
}

void fogStart(){
    DMX.write(fog_channel, 255);
}


void fogStop(){
    DMX.write(fog_channel, 0);
}
