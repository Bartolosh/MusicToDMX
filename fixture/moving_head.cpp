#include "moving_head.h"

void init_movinghead(moving_head *head, int start_address){
    head->start_address = start_address;
    head->ch_dimmer = start_address + 5;
    head->ch_color = start_address + 7;
    head->ch_strobe = start_address + 6;
    head->ch_pan = start_address;
    head->ch_tilt = start_address + 2;
    head->ch_speed = start_address + 4;
}

void set_color(moving_head head,int color){
    switch(color){
        case RED:
            DMX.write(head.ch_color, RED_VALUE);
            //DmxSimple.write(head.ch_color, 10);
            break;
        case BLUE:
            DMX.write(head.ch_color, BLUE_VALUE);
            break;
        case GREEN:
            DMX.write(head.ch_color, GREEN_VALUE);
            break;
        case PINK:
            DMX.write(head.ch_color, PINK_VALUE);
            break;
        case LIGHT_BLUE:
            DMX.write(head.ch_color, LIGHTBLUE_VALUE);
            break;
        default:
            DMX.write(head.ch_color, WHITE_VALUE);
            break;
    }

    DMX.write(head.ch_dimmer, 255);
}

void rotate(moving_head *head, int speed, int color){
    set_color(*head, color);
    //DmxSimple.write(head->ch_speed, speed);

    //TODO: controllare valori per posizioni
    switch(head->state){
        case ROTATE1:
            DMX.write(head->ch_pan, 114);
            DMX.write(head->ch_tilt, 207);
            head->state = ROTATE2;
            break;
        case ROTATE2:
            DMX.write(head->ch_pan, 89);
            DMX.write(head->ch_tilt, 229);
            head->state = ROTATE3;
            break;
        case ROTATE3:
            DMX.write(head->ch_pan, 69);
            DMX.write(head->ch_tilt, 216);
            head->state = ROTATE4;
            break;
        case ROTATE4:
            DMX.write(head->ch_pan, 87);
            DMX.write(head->ch_tilt, 208);
            head->state = ROTATE1;
            break;
        default:
            DMX.write(head->ch_pan, 87);
            DMX.write(head->ch_tilt, 208);
            head->state = ROTATE1;
            break;
    }
}

void strobe_head(moving_head head, int speed, int color){
    set_color(head, color);
    map(speed, 0, 255, 0, 128); //Controllare velocità strobe
    DMX.write(head.ch_strobe, speed);
}

void up_down(moving_head *head, int color, int speed){
    set_color(*head, color);

    DMX.write(head->ch_speed, speed);

    //TODO: controllare valori per posizioni
    switch(head->state){
        case UP1:
            DMX.write(head->ch_pan, 0);
            DMX.write(head->ch_tilt, 30);
            head->state = DOWN2;
            break;
        case DOWN2:
            DMX.write(head->ch_pan, 0);
            DMX.write(head->ch_tilt, 116);
            head->state = UP1;
            break;
        default:
            DMX.write(head->ch_pan, 0);
            DMX.write(head->ch_tilt, 40);
            head->state = UP1;
            break;
    }
}