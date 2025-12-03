#include <nrf.h>
#include "timer.h"
#include "board_config.h"
#include "gpio.h"

//=========================== swarmit ==========================================

void swarmit_keep_alive(void);
void swarmit_localization_handle_isr(void);
void SPIM4_IRQHandler(void) { swarmit_localization_handle_isr(); }
#define SWARMIT_TIMER_DEV (0)

//=========================== variables ========================================

static const gpio_t led_red   = { .port = DB_RGB_LED_PWM_RED_PORT,   .pin = DB_RGB_LED_PWM_RED_PIN   };
static const gpio_t led_green = { .port = DB_RGB_LED_PWM_GREEN_PORT, .pin = DB_RGB_LED_PWM_GREEN_PIN };
static const gpio_t led_blue  = { .port = DB_RGB_LED_PWM_BLUE_PORT,  .pin = DB_RGB_LED_PWM_BLUE_PIN  };

//=========================== main =============================================

int main(void) {
    // ---- swarmit setup ----
    db_timer_init(SWARMIT_TIMER_DEV);
    db_timer_set_periodic_ms(SWARMIT_TIMER_DEV, 0, 200, &swarmit_keep_alive);

    db_gpio_init(&led_red,   DB_GPIO_OUT);
    db_gpio_init(&led_green, DB_GPIO_OUT);
    db_gpio_init(&led_blue,  DB_GPIO_OUT);

    while (1) {
        // switch LEDs on
        db_gpio_set(&led_red);
        db_gpio_set(&led_green);
        db_gpio_set(&led_blue);
 
        // switch LEDs off
        db_gpio_clear(&led_red);
        db_gpio_clear(&led_green);
        db_gpio_clear(&led_blue);
    }
}
