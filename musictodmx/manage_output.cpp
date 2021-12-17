#include "./manage_output.h"

void send_output(int speed){
    eurolite_par par1;
    eurolite_par par2;
    eurolite_par par3;

    moving_head mov1;
    moving_head mov2; //todo control address

    init_eurolitepar(&par1, 1);
    init_eurolitepar(&par2, 7);
    init_eurolitepar(&par3, 13);

    init_movinghead(&mov1, 19);
    init_movinghead(&mov2, 41);

    //DmxSimple.maxChannel(70);

    if (!DMX.begin(70)) {
        return ;
    }

    int color = rand() %6 +1;

    int mov = rand() %2;

    int mov_col = rand() %3;

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

}
