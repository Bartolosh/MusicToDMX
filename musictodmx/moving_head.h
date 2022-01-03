/*
    struct and functions for manage moving head
*/

#ifndef MovingHead_h
#define MovingHead_h

#include <ArduinoRS485.h> // the ArduinoDMX library depends on ArduinoRS485
#include <ArduinoDMX.h>
#include "color.h"
#include <stdlib.h> //usata per random

#define HOLD  5

#define ROTATE    1
#define UP_DOWN   2


#define ROTATE1     1
#define ROTATE2     2
#define ROTATE3     3
#define ROTATE4     4

#define UP1     5
#define DOWN2   6

#define RED_VALUE       35
#define ORANGE_VALUE    50
#define WHITE_VALUE     0
#define BLUE_VALUE      70
#define GREEN_VALUE     90
#define YELLOW_VALUE    110
#define VIOLET_VALUE    125
#define LIGHTBLUE_VALUE 175
#define LIGHTGREEN_VALUE 140
#define PINK_VALUE      160
#define ROTATION_COLOR  220

typedef struct{
    int start_address;
    int ch_color;
    int ch_dimmer;
    int ch_strobe;
    int ch_pan; 
    int ch_tilt;
    int ch_speed;
    int state;
    int mov;
    uint8_t hold;
}moving_head;


void init_movinghead(moving_head *head, int start_address);

void set_color(moving_head head,uint8_t color);

void rotate(moving_head *head, uint8_t speed, uint8_t color);

void strobe_head(moving_head head, uint8_t speed, uint8_t color);

void up_down(moving_head *head, uint8_t color, uint8_t speed);

#endif
