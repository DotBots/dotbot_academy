#include <stdint.h>
#include <nrf.h>
// Include BSP packages
#include "board.h"
#include "motors.h"

//=========================== defines ==========================================

#define TIMER_DEV (0)

//=========================== main =============================================

int main(void) {
    // Turn ON the DotBot board regulator
    db_board_init();

    // Configure Motors
    db_motors_init();

    while (1) {
        // Move forward
        db_motors_set_speed(50, 50);
    }
}
