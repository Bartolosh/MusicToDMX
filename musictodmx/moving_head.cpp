#include "moving_head.h"

void init_movinghead(moving_head *head, int start_address){
    head->start_address = start_address;
    head->ch_dimmer = start_address + 3;
    head->ch_color = start_address + 5;
    head->ch_strobe = start_address + 6;
    head->ch_pan = start_address;
    head->ch_tilt = start_address + 2;
    head->ch_speed = start_address + 4;
}

void set_color(moving_head head,int color){
    switch(color){
        case RED:
            DMX.beginTransmission();
            DMX.write(head.ch_color, 10);
            DMX.endTransmission();
            //DmxSimple.write(head.ch_color, 10);
            break;
        case BLUE:
            DMX.beginTransmission();
            DMX.write(head.ch_color, 20);
            DMX.endTransmission();
            //DmxSimple.write(head.ch_color, 20);
            break;
        case GREEN:
            //DmxSimple.write(head.ch_color, 30);
            break;
        case PINK:
            //DmxSimple.write(head.ch_color, 40);
            break;
        case LIGHT_BLUE:
            //DmxSimple.write(head.ch_color, 50);
            break;
        default:
            //DmxSimple.write(head.ch_color, 0);
            break;
    }

    //DmxSimple.write(head.ch_dimmer, 255);
}

void rotate(moving_head *head, int speed, int color){
    set_color(*head, color);
    //DmxSimple.write(head->ch_speed, speed);

    //TODO: controllare valori per posizioni
    switch(head->state){
        case ROTATE1:
            //DmxSimple.write(head->ch_pan, 20);
            //DmxSimple.write(head->ch_tilt, 40);
            head->state = ROTATE2;
            break;
        case ROTATE2:
            //DmxSimple.write(head->ch_pan, 20);
            //DmxSimple.write(head->ch_tilt, 40);
            head->state = ROTATE3;
            break;
        case ROTATE3:
            //DmxSimple.write(head->ch_pan, 20);
            //DmxSimple.write(head->ch_tilt, 40);
            head->state = ROTATE4;
            break;
        case ROTATE4:
            //DmxSimple.write(head->ch_pan, 20);
            //DmxSimple.write(head->ch_tilt, 40);
            head->state = ROTATE1;
            break;
        default:
            //DmxSimple.write(head->ch_pan, 20);
            //DmxSimple.write(head->ch_tilt, 40);
            head->state = ROTATE1;
            break;
    }
}

void strobe_head(moving_head head, int speed, int color){
    set_color(head, color);
    map(speed, 0, 255, 0, 128); //Controllare velocità strobe
    //DmxSimple.write(head.ch_strobe, speed);
}

void up_down(moving_head *head, int color, int speed){
    set_color(*head, color);

    //DmxSimple.write(head->ch_speed, speed);

    //TODO: controllare valori per posizioni
    switch(head->state){
        case UP1:
            //DmxSimple.write(head->ch_pan, 20);
            //DmxSimple.write(head->ch_tilt, 40);
            head->state = DOWN2;
            break;
        case DOWN2:
            //DmxSimple.write(head->ch_pan, 128);
            //DmxSimple.write(head->ch_tilt, 40);
            head->state = UP1;
            break;
        default:
            //DmxSimple.write(head->ch_pan, 128);
            //DmxSimple.write(head->ch_tilt, 40);
            head->state = UP1;
            break;
    }
}