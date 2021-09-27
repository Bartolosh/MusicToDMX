/*
    File for manage output for par led rgb Eurolite cob100
*/

#ifndef Movinghead_h
#define Movinghead_h
#include <DmxSimple.h>
#include "color.h"
#include <stdlib.h> //da vedere se serve... troppo pesa temo

#define ROTATE1     1
#define ROTATE2     2
#define ROTATE3     3
#define ROTATE4     4

#define UP1     5
#define DOWN2     6

class MovingHead{

    public:
        MovingHead(int address);
        void set_color(int color);
        void rotate(int color = WHITE, int speed);
        void strobe(int speed, int color = WHITE);
        void up_down(int color = WHITE, int speed);
        
    private:
        int start_address;
        int ch_color;
        int ch_dimmer;
        int ch_strobe;
        int ch_pan; 
        int ch_tilt;
        int ch_speed;
        int state;
}

#endif