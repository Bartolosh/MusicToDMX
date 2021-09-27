/*
    File for manage output for par led rgb Eurolite cob100
*/

#ifndef EurolitePar_h
#define EurolitePar_h

#include <DmxSimple.h>
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

void change_color(eurolite_par par,int color);
void strobe(eurolite_par par, int speed, int color);
void rainbow(eurolite_par par);

#endif