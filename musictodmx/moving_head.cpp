#include "moving_head.h"

void init_movinghead(moving_head *head, int start_address){
    head->start_address = start_address;
    head->ch_dimmer = start_address + 5;
    head->ch_color = start_address + 7;
    head->ch_strobe = start_address + 6;
    head->ch_pan = start_address;
    head->ch_tilt = start_address + 2;
    head->ch_speed = start_address + 4;
    head->state = ROTATE1;
    head->mov = ROTATE;
    head->hold = 0;
}

void set_color(moving_head head,uint8_t color){
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

    DMX.write(head.ch_dimmer, 100);
}

void rotate(moving_head *head, uint8_t color, uint8_t speed){
    set_color(*head, color);
    DMX.write(head->ch_speed, speed);
    
    //TODO: controllare valori per posizioni
    switch(head->state){
        case ROTATE1:
            DMX.write(head->ch_pan, 114);
            DMX.write(head->ch_tilt, 207);
            if(head->hold <HOLD){
              head->hold ++;
            }
            else{
              head->hold = 0;
              head->state = ROTATE2;
            }
            break;
        case ROTATE2:
            DMX.write(head->ch_pan, 89);
            DMX.write(head->ch_tilt, 229);
            if(head->hold <HOLD){
              head->hold ++;
            }
            else{
              head->hold = 0;
              head->state = ROTATE3;
            }
            break;
        case ROTATE3:
            DMX.write(head->ch_pan, 69);
            DMX.write(head->ch_tilt, 216);
            if(head->hold <HOLD){
              head->hold ++;
            }
            else{
              head->hold = 0;
              head->state = ROTATE4;
            }
            break;
        case ROTATE4:
            DMX.write(head->ch_pan, 87);
            DMX.write(head->ch_tilt, 208);
            if(head->hold <HOLD){
              head->hold ++;
            }
            else{
              head->hold = 0;
              head->state = ROTATE1;
            }
            break;
        default:
            DMX.write(head->ch_pan, 87);
            DMX.write(head->ch_tilt, 208);
            if(head->hold <HOLD){
              head->hold ++;
            }
            else{
              head->hold = 0;
              head->state = ROTATE1;
            }

            break;
    }
}

void strobe_head(moving_head head, uint8_t speed, uint8_t color){
    set_color(head, color);
    map(speed, 0, 255, 0, 128); //Controllare velocità strobe
    DMX.write(head.ch_strobe, speed);
}

void up_down(moving_head *head, uint8_t color, uint8_t speed){
    set_color(*head, color);

    DMX.write(head->ch_speed, (speed));

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

void sx_dx(moving_head *head, uint8_t color, uint8_t speed){
    set_color(*head, color);

    DMX.write(head->ch_speed, speed);

    switch(head->state){
        case SX:
            DMX.write(head->ch_pan, 162);
            DMX.write(head->ch_tilt, 31);
            head->state = DX;
            break;
        case DX:
            DMX.write(head->ch_pan, 202);
            DMX.write(head->ch_tilt, 31);
            head->state = SX;
            break;
        default:
            DMX.write(head->ch_pan, 162);
            DMX.write(head->ch_tilt, 31);
            head->state = DX;
            break;
    }
}


void mov_v(moving_head *head, uint8_t color, uint8_t speed){
    set_color(*head, color);

    DMX.write(head->ch_speed, speed);

    switch(head->state){
        case V1:
            DMX.write(head->ch_pan, 153);
            DMX.write(head->ch_tilt, 39);
            if(head->hold <HOLD){
              head->hold ++;
            }
            else{
              head->hold = 0;
              head->state = V2;
            }
            break;
        case V2:
            DMX.write(head->ch_pan, 162);
            DMX.write(head->ch_tilt, 100);
            if(head->hold <HOLD){
              head->hold ++;
            }
            else{
              head->hold = 0;
              head->state = V3;
            }
            break;
        case V3:
            DMX.write(head->ch_pan, 178);
            DMX.write(head->ch_tilt, 35);
            
            if(head->hold <HOLD){
              head->hold ++;
            }
            else{
              head->hold = 0;
              head->state = V4;
            }
            break;
        case V4:
            DMX.write(head->ch_pan, 162);
            DMX.write(head->ch_tilt, 100);
            if(head->hold <HOLD){
              head->hold ++;
            }
            else{
              head->hold = 0;
              head->state = V1;
            }
            break;
        default:
            DMX.write(head->ch_pan, 153);
            DMX.write(head->ch_tilt, 39);
            if(head->hold <HOLD){
              head->hold ++;
            }
            else{
              head->hold = 0;
              head->state = V2;
            }
            break;
    }
}
