#include "eurolite_par.h"

void init_eurolitepar(eurolite_par *par, int start_address){
    par->start_address = start_address;
    par->ch_red = start_address;
    par->ch_blue = start_address+2;
    par->ch_green = start_address+1;
    par->ch_dimmer = start_address+4;
    par->ch_strobe = start_address+5;
}

void change_color(eurolite_par par,int color){
    switch(color){
        case RED:
            DMX.write(par.ch_red, 255);
            DMX.write(par.ch_blue, 0);
            DMX.write(par.ch_green,0);
            break;
        case BLUE:
            DMX.write(par.ch_red, 0);
            DMX.write(par.ch_blue, 255);
            DMX.write(par.ch_green,0);
            break;
        case GREEN:
            DMX.write(par.ch_red, 0);
            DMX.write(par.ch_blue, 0);
            DMX.write(par.ch_green,255);
            break;
        case PINK:
            DMX.write(par.ch_red, 255);
            DMX.write(par.ch_blue, 255);
            DMX.write(par.ch_green,0);
            break;
        case LIGHT_BLUE:
            DMX.write(par.ch_red, 0);
            DMX.write(par.ch_blue, 255);
            DMX.write(par.ch_green,255);
            break;
        default:
            DMX.write(par.ch_red, 255);
            DMX.write(par.ch_blue, 255);
            DMX.write(par.ch_green,255);
            break;
    }

    DMX.write(par.ch_dimmer, 255);
}
void strobe_par(eurolite_par par, int speed, int color){
    change_color(par, color);
    map(speed,0,255,0,200); //da controllare massimo valore prima del random
    DMX.write(par.ch_strobe, speed);
}
void rainbow(eurolite_par par){
    int color = rand() %6 +1; //generate a random number from 1 to 6
    change_color(par, color);
}