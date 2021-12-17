#include "fog.h"

void init_fog(uint8_t ch){
    channel = ch;
}

TaskHandle_t taskFogHandle = NULL;

void fogHandler(void) {
  BaseType_t xYieldRequired = xTaskResumeFromISR(taskFogHandle);   /* INCLUDE_vTaskSuspend and INCLUDE_xTaskResumeFromISR must be defined as 1 */
  portYIELD_FROM_ISR(xYieldRequired);
}

void fogSelector() {
  pinMode(FOG_BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(FOG_BUTTON_PIN), fogHandler, FALLING);
}
