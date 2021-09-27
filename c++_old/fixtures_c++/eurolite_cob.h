/*
    File for manage output for par led rgb Eurolite cob100
*/

#ifndef EurolitePar_h
#define EurolitePar_h
#include <DmxSimple.h>
#include "color.h"
#include <stdlib.h> //da vedere se serve... troppo pesa temo

class EurolitePar{

    public:
        EurolitePar(int address);
        void change_color(int color = RED);
        void strobe(int speed, int color = WHITE);
        void rainbow();
        
    private:
        int start_address;
        int ch_red;
        int ch_blue;
        int ch_green;
        int ch_dimmer;
        int ch_strobe;
}

#endif