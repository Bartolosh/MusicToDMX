#ifndef FIRECONTROLLER_H
#define FIRECONTROLLER_H

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <ArduinoRS485.h> 
#include <ArduinoDMX.h>

#define FIRE_BUTTON_PIN D4
#define FIRE_DURATION   50 /*flame's duration [0-255]*/

extern uint16_t fire_channel;
extern uint16_t fire_channel_duration;
extern TaskHandle_t taskFireHandle;

extern void init_fire(uint16_t channel);
extern void fireHandler(void);
extern void fireSelector(void);

extern void fireStart(void);
extern void fireStop(void);

#endif
