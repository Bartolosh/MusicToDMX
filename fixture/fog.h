#ifndef FOGCONTROLLER_H
#define FOGCONTROLLER_H

#include <arduino.h>
#include <STM32FreeRTOS.h>

#define FOG_BUTTON_PIN 13

extern uint8_t channel= 0;
extern TaskHandle_t taskFogHandle;
extern void fogHandler(void);
extern void fogSelector();

#endif