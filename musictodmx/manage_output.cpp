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

// mode = 1 --> random 
// mode = 0 --> use current
void send_output(uint8_t speed, uint8_t light_mode, uint8_t mov_mode, uint8_t fog_state, uint8_t fire_start){
    uint8_t color;
    int mov;

    speed = map(speed, 70, 170, 150, 90);
    // check if need to change light
    if(light_mode){
        //Serial.println("change color");
        color = rand() % 6 +1;
        if(color == par1.current_color ){
          color = (color + 1)% 6 +1;
        }
        par1.current_color = color;
    }
    else{
        //Serial.println("no change");
        color = par1.current_color;
    }
    
    // check if need to change mov
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

    int mov_col = 0;

    DMX.beginTransmission();

    if(fog_state == 1){
          DMX.write(57,255);
    }
    else{
      
      DMX.write(57,0);
    }

    if(fire_start == 1){
      DMX.write(2,50);
      DMX.write(1,255);
    }
    else{
      
      DMX.write(1,0);
    }

    switch(mov_col){
        case 0:
            change_color(par1, color);
            change_color(par2, color);
            change_color(par3, color);
            break;
        case 1:
            rainbow(par1);
            rainbow(par2);
            rainbow(par3);
            break;
        case 2:
            strobe_par(par1, speed,color);
            strobe_par(par2, speed,color);
            strobe_par(par3, speed,color);
            break;
        default:
            change_color(par1 ,color);
            change_color(par2 ,color);
            change_color(par3 ,color);
            break;
    }

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
