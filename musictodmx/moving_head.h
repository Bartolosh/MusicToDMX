/*
    struct and functions for manage moving head
*/

#ifndef MovingHead_h
#define MovingHead_h

#include <ArduinoRS485.h> // the ArduinoDMX library depends on ArduinoRS485
#include <ArduinoDMX.h>
#include "color.h"
#include <stdlib.h> //usata per random


#define ROTATE1     1
#define ROTATE2     2
#define ROTATE3     3
#define ROTATE4     4

#define UP1     5
#define DOWN2   6

typedef struct{
    int start_address;
    int ch_color;
    int ch_dimmer;
    int ch_strobe;
    int ch_pan; 
    int ch_tilt;
    int ch_speed;
    int state;
}moving_head;


void init_movinghead(moving_head *head, int start_address);

void set_color(moving_head head,int color);

void rotate(moving_head *head, int speed, int color);

void strobe_head(moving_head head, int speed, int color);

void up_down(moving_head *head, int color, int speed);

#endif