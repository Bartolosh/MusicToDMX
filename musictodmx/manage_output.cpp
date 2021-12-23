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

void send_output(uint8_t speed){

    uint8_t color = rand() %6 +1;

    int mov = 0;

    int mov_col = 0;
    Serial.println((String)"color = " + color + " mov = " + mov + " mov_col = " + mov_col);

    DMX.beginTransmission();

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
        default:
            rotate(&mov1, color, speed);
            rotate(&mov2, color,speed);
            break;
    }

    DMX.endTransmission();
}
