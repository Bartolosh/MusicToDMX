#include "./manage_output.h"


eurolite_par par1;
eurolite_par par2;
eurolite_par par3;

moving_head mov1;
moving_head mov2; 


void init_fixture(void){

    init_fire(1);
    init_eurolitepar(&par1, 3);
    init_eurolitepar(&par2, 9);
    init_eurolitepar(&par3, 15);

    init_movinghead(&mov1, 21);
    init_movinghead(&mov2, 39);

    init_fog(57);
    
    
    if (!DMX.begin(58)) {
        return ;
    }

}

void send_output(uint8_t speed, uint8_t light_mode, uint8_t mov_mode, uint8_t fog_state, uint8_t fire_start){
    uint8_t color;
    uint8_t mov;

    /* map bpm to speed for the movement */
    speed = map(speed, 70, 170, 150, 90);
   
    if(light_mode){
        color = rand() % 6 +1;
        if(color == par1.current_color ){
          color = (color + 1)% 6 +1;
        }
        par1.current_color = color;
    }
    else{
        color = par1.current_color;
    }
    
    /* check if need to change mov */
    if(mov_mode){
        mov = rand() % 4 +1;
        if(mov == mov1.mov ){
          mov = (mov + 1)% 6 +1;
        }
        mov1.mov = mov;
    }
    else{
        mov = mov1.mov;
    }

    DMX.beginTransmission();

    if(fog_state == 1){
          fogStart();
    }
    else{
      
      fogStop();
    }

    if(fire_start == 1){
      fireStart();
    }
    else{
      
      fireStop();
    }

    change_color(par1, color);
    change_color(par2, color);
    change_color(par3, color);    

    switch(mov){
        case 0:
            rotate(&mov1, color, speed);
            rotate(&mov2, color,speed);
            break;
        case 1:
            up_down(&mov1, color, speed);
            up_down(&mov2, color,speed);
            break;
        case 2:
            sx_dx(&mov2, color,speed);
            sx_dx(&mov1, color,speed);
            break;
        case 3:
            mov_v(&mov2, color,speed);
            mov_v(&mov1, color,speed);
            break;
        default:
            rotate(&mov1, color, speed);
            rotate(&mov2, color,speed);
            break;
    }

    DMX.endTransmission();
}
