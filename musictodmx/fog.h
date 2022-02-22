#ifndef FOGCONTROLLER_H
#define FOGCONTROLLER_H

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <ArduinoRS485.h> // the ArduinoDMX library depends on ArduinoRS485
#include <ArduinoDMX.h>

#define FOG_BUTTON_PIN      D3
#define FOG_DURATION_TIME   1500 //milliseconds

#define START   1
#define STOP    0

extern uint16_t fog_channel;
extern TaskHandle_t taskFogHandle;

extern void init_fog(uint16_t channel);
extern void fogHandler(void);
extern void fogSelector(void);

extern void fogStart(void);
extern void fogStop(void);

#endif
