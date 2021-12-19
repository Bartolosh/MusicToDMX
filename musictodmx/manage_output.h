#include "eurolite_par.h"
#include "moving_head.h"
#include "fire.h"
#include "fog.h"

#include <stdlib.h>
#include <ArduinoRS485.h> // the ArduinoDMX library depends on ArduinoRS485
#include <ArduinoDMX.h>

void send_output(int speed);

void init_fixture();
