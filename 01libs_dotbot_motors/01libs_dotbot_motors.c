#include <stdint.h>
#include <nrf.h>
#include "board.h"
#include "motors.h"

int main(void) {
    // turn on the DotBot board regulator
    db_board_init();

    // configure motors
    db_motors_init();

    while (1) {
        // move forward
        db_motors_set_speed(50, 50);
    }
}
