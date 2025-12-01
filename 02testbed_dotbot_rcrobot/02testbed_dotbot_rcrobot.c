#include <nrf.h>
#include <stdint.h>
#include <stdlib.h>
#include "motors.h"
#include "timer.h"
#include "board.h"
#include "board_config.h"
#include "gpio.h"

//=========================== defines ==========================================

#define TIMER_DEV           (0)
#define CMD_FORWARD         'F'
#define CMD_LEFT            'L'
#define CMD_RIGHT           'R'
#define CMD_STOP            'S'
#define SPEED               (75)
#define TIMEOUT_CHECK_MS    (200)
#define TIMEOUT_CHECK_TICKS (25000)  ///< delay between packet received timeout checks

typedef struct {
    uint32_t ts_last_packet_received; ///< Last time a control packet was received
} dotbot_vars_t;

//=========================== variables ========================================

static dotbot_vars_t _dotbot_vars;

static const gpio_t led_red   = { .port = DB_RGB_LED_PWM_RED_PORT,   .pin = DB_RGB_LED_PWM_RED_PIN   };
static const gpio_t led_green = { .port = DB_RGB_LED_PWM_GREEN_PORT, .pin = DB_RGB_LED_PWM_GREEN_PIN };
static const gpio_t led_blue  = { .port = DB_RGB_LED_PWM_BLUE_PORT,  .pin = DB_RGB_LED_PWM_BLUE_PIN  };

//=========================== callbacks ========================================

static void _rx_data_callback(const uint8_t *pkt, size_t len) {
    (void)len;
    _dotbot_vars.ts_last_packet_received = db_timer_ticks(TIMER_DEV);
    switch (*pkt) {
        case CMD_FORWARD:
            db_motors_set_speed(SPEED, SPEED);
            db_gpio_set(&led_red); db_gpio_clear(&led_green); db_gpio_clear(&led_blue);
            break;
        case CMD_LEFT:
            db_motors_set_speed(-SPEED, SPEED);
            db_gpio_clear(&led_red); db_gpio_set(&led_green); db_gpio_clear(&led_blue);
            break;
        case CMD_RIGHT:
            db_motors_set_speed(SPEED, -SPEED);
            db_gpio_clear(&led_red); db_gpio_clear(&led_green); db_gpio_set(&led_blue);
            break;
        case CMD_STOP:
            db_motors_set_speed(0, 0);
            db_gpio_set(&led_red); db_gpio_set(&led_green); db_gpio_set(&led_blue);
            break;
    }
}

static void _timeout_check(void) {
    uint32_t ticks = db_timer_ticks(TIMER_DEV);
    if (ticks > _dotbot_vars.ts_last_packet_received + TIMEOUT_CHECK_TICKS) {
        db_motors_set_speed(0, 0);
        db_gpio_clear(&led_red); db_gpio_clear(&led_green); db_gpio_clear(&led_blue);
    }
}

//=========================== swarmit ==========================================

void swarmit_keep_alive(void);
void swarmit_localization_handle_isr(void);
typedef void (*ipc_isr_cb_t)(const uint8_t *, size_t);
void swarmit_ipc_isr(ipc_isr_cb_t cb);
void SPIM4_IRQHandler(void) { swarmit_localization_handle_isr(); }
void IPC_IRQHandler(void) { swarmit_ipc_isr(_rx_data_callback); }

//=========================== main =============================================

int main(void) {
    // ---- swarmit setup ----
    db_timer_init(TIMER_DEV);
    db_timer_set_periodic_ms(TIMER_DEV, 0, 200, &swarmit_keep_alive);

    // initialize peripherals
    db_board_init();
    db_motors_init();

    db_gpio_init(&led_red,   DB_GPIO_OUT);
    db_gpio_init(&led_green, DB_GPIO_OUT);
    db_gpio_init(&led_blue,  DB_GPIO_OUT);

    db_timer_set_periodic_ms(TIMER_DEV, 1, TIMEOUT_CHECK_MS, &_timeout_check);

    while (1) {
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event
    }
}
