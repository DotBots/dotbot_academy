#include <stdint.h>
#include <nrf.h>
#include "motors.h"
#include "timer.h"
#include "board.h"

//=========================== swarmit ==========================================

void swarmit_keep_alive(void);
void swarmit_localization_handle_isr(void);
void SPIM4_IRQHandler(void) { swarmit_localization_handle_isr(); }
#define SWARMIT_TIMER_DEV (0)

//=========================== main =============================================

int main(void) {
    // ---- swarmit setup ----
    db_timer_init(SWARMIT_TIMER_DEV);
    db_timer_set_periodic_ms(SWARMIT_TIMER_DEV, 0, 200, &swarmit_keep_alive);

    // turn on the DotBot board regulator
    db_board_init();

    // configure motors
    db_motors_init();

    while (1) {
        // move forward
        db_motors_set_speed(50, 50);
    }
}
