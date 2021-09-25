#include "eurolite_cob.h"

EurolitePar::EurolitePar(int address){
    start_address = address;
    ch_red = start_address;
    ch_blue = start_address+1;
    ch_green = start_address+2;
    ch_dimmer = start_address+4;
    ch_strobe = start_address+5;
}

EurolitePar::change_color(int color = RED){
    switch(color){
        case RED:
            DmxSimple.write(ch_red, 255);
            DmxSimple.write(ch_blue, 0);
            DmxSimple.write(ch_green,0);
            break;
        case BLUE;
            DmxSimple.write(ch_red, 0);
            DmxSimple.write(ch_blue, 255);
            DmxSimple.write(ch_green,0);
            break;
        case GREEN:
            DmxSimple.write(ch_red, 0);
            DmxSimple.write(ch_blue, 0);
            DmxSimple.write(ch_green,255);
            break;
        case PINK:
            DmxSimple.write(ch_red, 255);
            DmxSimple.write(ch_blue, 255);
            DmxSimple.write(ch_green,0);
            break;
        case LIGHT_BLUE:
            DmxSimple.write(ch_red, 0);
            DmxSimple.write(ch_blue, 255);
            DmxSimple.write(ch_green,255);
            break;
        default:
            DmxSimple.write(ch_red, 255);
            DmxSimple.write(ch_blue, 255);
            DmxSimple.write(ch_green,255);
            break;
    }

    DmxSimple.write(ch_dimmer, 255);

}

EurolitePar::strobe(int speed, int color = WHITE){
    change_color(color);
    map(speed,0,255,0,200); //da controllare massimo valore prima del random
    DmxSimple.write(ch_strobe, speed);
}

EurolitePar::rainbow(){
    int color = rand() %6 +1; //generate a random number from 1 to 6
    change_color(color);
}