#ifndef FIRECONTROLLER_H
#define FIRECONTROLLER_H

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <ArduinoRS485.h> // the ArduinoDMX library depends on ArduinoRS485
#include <ArduinoDMX.h>

#define FIRE_BUTTON_PIN 14
#define FIRE_DURATION   34 //flame's duration [0-255]

extern uint8_t fire_channel;
extern uint8_t fire_channel_duration;
extern TaskHandle_t taskFireHandle;

extern void init_fire(uint8_t channel);
extern void fireHandler(void);
extern void fireSelector();

extern void fireStart();

#endif
