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
    uint16_t start_address;
    uint16_t ch_red;
    uint16_t ch_blue;
    uint16_t ch_green;
    uint16_t ch_dimmer;
    uint16_t ch_strobe;
    uint8_t current_color;
    uint8_t current_mov;
}eurolite_par;

void init_eurolitepar(eurolite_par *par, uint16_t start_address);

void change_color(eurolite_par par,uint8_t color);

#endif
