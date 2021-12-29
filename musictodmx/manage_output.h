#include "eurolite_par.h"
#include "moving_head.h"
#include "fire.h"
#include "fog.h"

#include <stdlib.h>
#include <ArduinoRS485.h> // the ArduinoDMX library depends on ArduinoRS485
#include <ArduinoDMX.h>

extern eurolite_par par1;
extern eurolite_par par2;
extern eurolite_par par3;

extern moving_head mov1;
extern moving_head mov2; //todo control address


void send_output(uint8_t speed, uint8_t light_mode, uint8_t mov_mode);

void init_fixture(void);
