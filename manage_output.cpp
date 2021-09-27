#include "./fixtures/eurolite_cob.h"
#include "./fixtures/moving_head.h"

#include <stdlib.h>

void send_output(int speed){
    EurolitePar par1 = EurolitePar(1);
    EurolitePar par2 = EurolitePar(7);
    EurolitePar par3 = EurolitePar(13);

    MovingHead mov1 = MovingHead(19);
    MovingHead mov2 = MovingHead(41); //todo control address

    int color = rand() %6 +1;

    int mov = rand() %2;

    int mov_col = rand() %3;

    switch(mov_col){
        case 0:
            par1.change_color(color);
            par2.change_color(color);
            par3.change_color(color);
            break;
        case 1:
            par1.rainbow();
            par2.rainbow();
            par3.rainbow();
            break;
        case 2:
            par1.strobe(speed,color);
            par2.strobe(speed,color);
            par3.strobe(speed,color);
            break;
        default:
            par1.change_color(color);
            par2.change_color(color);
            par3.change_color(color);
            break;
    }

    switch(mov){
        case 0:
            mov1.rotate(color, speed);
            mov2.rotate(color,speed);
            break;
        case 1:
            mov1.up_down(color, speed);
            mov2.up_down(color,speed);
            break;
        default:
            mov1.rotate(color, speed);
            mov2.rotate(color,speed);
            break;
    }

}
