#ifndef FOGCONTROLLER_H
#define FOGCONTROLLER_H

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <ArduinoRS485.h> // the ArduinoDMX library depends on ArduinoRS485
#include <ArduinoDMX.h>

#define FOG_BUTTON_PIN      13
#define FOG_DURATION_TIME   2000 //milli seconds

extern uint8_t fog_channel;
extern TaskHandle_t taskFogHandle;

extern void init_fog(uint8_t channel);
extern void fogHandler(void);
extern void fogSelector();

extern void fogStart();
extern void fogStop();

#endif
