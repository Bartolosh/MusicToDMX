#include "eurolite_par.h"

#define ROTATE 0
#define UPNDOWN 1

void init_eurolitepar(eurolite_par *par, int start_address){
    par->start_address = start_address;
    par->ch_red = start_address;
    par->ch_blue = start_address+2;
    par->ch_green = start_address+1;
    par->ch_dimmer = start_address+4;
    par->ch_strobe = start_address+5;
    par->current_color = WHITE;
}

void change_color(eurolite_par par,uint8_t color){
  
    switch(color){
        case RED:
            DMX.write(par.ch_red, 255);
            DMX.write(par.ch_blue, 0);
            DMX.write(par.ch_green,0);
            par.current_color = RED;
            break;
        case BLUE:
            DMX.write(par.ch_red, 0);
            DMX.write(par.ch_blue, 255);
            DMX.write(par.ch_green,0);
            par.current_color = BLUE;
            break;
        case GREEN:
            DMX.write(par.ch_red, 0);
            DMX.write(par.ch_blue, 0);
            DMX.write(par.ch_green,255);
            par.current_color = GREEN;
            break;
        case PINK:
            DMX.write(par.ch_red, 255);
            DMX.write(par.ch_blue, 255);
            DMX.write(par.ch_green,0);
            par.current_color = PINK;
            break;
        case LIGHT_BLUE:
            DMX.write(par.ch_red, 0);
            DMX.write(par.ch_blue, 255);
            DMX.write(par.ch_green,255);
            par.current_color = LIGHT_BLUE;
            break;
        default:
            DMX.write(par.ch_red, 255);
            DMX.write(par.ch_blue, 255);
            DMX.write(par.ch_green,255);
            par.current_color = WHITE;
            break;
    }

    DMX.write(par.ch_dimmer, 100);
}
