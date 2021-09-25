#include "moving_head.h"


MovingHead::MovingHead(int address) {
    start_address = address;
    ch_dimmer = start_address + 3;
    ch_color = start_address + 5;
    ch_strobe = start_address + 6;
    ch_pan = start_address;
    ch_tilt = start_address + 2;
    ch_speed = start_address + 4;
}

void MovingHead::set_color(int color){
    switch(color){
        case RED:
            DmxSimple.write(ch_color, 10);
            break;
        case BLUE:
            DmxSimple.write(ch_color, 20);
            break;
        case GREEN:
            DmxSimple.write(ch_color, 30);
            break;
        case PINK:
            DmxSimple.write(ch_color, 40);
            break;
        case LIGHT_BLUE:
            DmxSimple.write(ch_color, 50);
            break;
        default:
            DmxSimple.write(ch_color, 0);
            break;
    }

    DmxSimple.write(ch_dimmer, 255);
}

void MovingHead::rotate(int color = WHITE, int speed){
    set_color(color);
    DmxSimple.write(ch_speed, speed);

    //TODO: controllare valori per posizioni
    switch(state){
        case ROTATE1:
            DmxSimple.write(ch_pan, 20);
            DmxSimple.write(ch_tilt, 40);
            state = ROTATE2;
            break;
        case ROTATE2:
            DmxSimple.write(ch_pan, 20);
            DmxSimple.write(ch_tilt, 40);
            state = ROTATE3;
            break;
        case ROTATE3:
            DmxSimple.write(ch_pan, 20);
            DmxSimple.write(ch_tilt, 40);
            state = ROTATE4;
            break;
        case ROTATE4:
            DmxSimple.write(ch_pan, 20);
            DmxSimple.write(ch_tilt, 40);
            state = ROTATE1;
            break;
        default:
            DmxSimple.write(ch_pan, 20);
            DmxSimple.write(ch_tilt, 40);
            state = ROTATE1;
            break;
    }
}

void MovingHead::strobe(int speed, int color = WHITE){
    set_color(color);
    map(speed, 0, 255, 0, 128); //Controllare velocità strobe
    DmxSimple.write(ch_strobe, speed)
}

void MovingHead::up_down(int color = WHITE, int speed){
    set_color(color);

    DmxSimple.write(ch_speed, speed);

    //TODO: controllare valori per posizioni
    switch(state){
        case UP1:
            DmxSimple.write(ch_pan, 20);
            DmxSimple.write(ch_tilt, 40);
            state = DOWN2;
            break;
        case DOWN2:
            DmxSimple.write(ch_pan, 20);
            DmxSimple.write(ch_tilt, 40);
            state = UP1;
            break;
        default:
            DmxSimple.write(ch_pan, 20);
            DmxSimple.write(ch_tilt, 40);
            state = UP1;
            break;
    }
}
