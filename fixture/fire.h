#ifndef FIRECONTROLLER_H
#define FIRECONTROLLER_H

#include <arduino.h>
#include <STM32FreeRTOS.h>

#define FIRE_BUTTON_PIN 14

extern uint8_t channel= 0;
extern TaskHandle_t taskFireHandle;
extern void fireHandler(void);
extern void fireSelector();

#endif