#include "fire.h"

uint8_t fire_channel = 0;

void init_fire(uint8_t ch){
    fire_channel = ch;
}

TaskHandle_t taskFireHandle = NULL;

//function that manage interrupt
void fireHandler(void) {
  //this function awake the task 
  BaseType_t xYieldRequired = xTaskResumeFromISR(taskFireHandle);   /* INCLUDE_vTaskSuspend and INCLUDE_xTaskResumeFromISR must be defined as 1 */
  portYIELD_FROM_ISR(xYieldRequired);
}

void fireSelector() {
  pinMode(FIRE_BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(FIRE_BUTTON_PIN), fireHandler, FALLING);
}


void fireStart(){
    DMX.beginTransmission();
    DMX.write(fire_channel, 255);
    DMX.endTransmission();
    DMX.beginTransmission();
    DMX.write(fire_channel, 0);
    DMX.endTransmission();
}
