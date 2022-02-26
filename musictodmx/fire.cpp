#include "fire.h"

uint16_t fire_channel = 1;

void init_fire(uint16_t ch){
    fire_channel = ch;
}

TaskHandle_t taskFireHandle = NULL;

/*function that manage interrupt*/
void fireHandler(void) {
  /*this function awake the task*/ 
  BaseType_t xYieldRequired = xTaskResumeFromISR(taskFireHandle);   /* INCLUDE_vTaskSuspend and INCLUDE_xTaskResumeFromISR must be defined as 1 */
  portYIELD_FROM_ISR(xYieldRequired);
}

void fireSelector(void) {
  pinMode(FIRE_BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(FIRE_BUTTON_PIN), fireHandler, FALLING);
}


void fireStart(void){
    DMX.write(fire_channel, 255);
    DMX.write(fire_channel+1, FIRE_DURATION);    
}

void fireStop(void){
    DMX.write(fire_channel, 0);
}
