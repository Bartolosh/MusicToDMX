/*
    File for manage output for par led rgb Eurolite cob100
*/

#ifndef EurolitePar_h
#define EurolitePar_h

#include <ArduinoRS485.h> // the ArduinoDMX library depends on ArduinoRS485
#include <ArduinoDMX.h>
#include "color.h"
#include <stdlib.h>

typedef struct{
    int start_address;
    int ch_red;
    int ch_blue;
    int ch_green;
    int ch_dimmer;
    int ch_strobe;
}eurolite_par;

void init_eurolitepar(eurolite_par *par, int start_address);

void change_color(eurolite_par par,uint8_t color);
void strobe_par(eurolite_par par, uint8_t speed, uint8_t color);
void rainbow(eurolite_par par);

#endif