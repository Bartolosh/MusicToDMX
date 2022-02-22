#ifndef MANAGEOUT_H
#define MANAGEOUT_H

#include "eurolite_par.h"
#include "moving_head.h"
#include "fire.h"
#include "fog.h"

#include <ArduinoRS485.h>
#include <ArduinoDMX.h>

extern eurolite_par par1;
extern eurolite_par par2;
extern eurolite_par par3;

extern moving_head mov1;
extern moving_head mov2; 


void send_output(uint8_t speed, uint8_t light_mode, uint8_t mov_mode, uint8_t fog_state, uint8_t fire_start);

void init_fixture(void);

#endif
