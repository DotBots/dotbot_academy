#include <nrf.h>
#include "board_config.h"
#include "gpio.h"

static const gpio_t led_red   = { .port = DB_RGB_LED_PWM_RED_PORT,   .pin = DB_RGB_LED_PWM_RED_PIN   };
static const gpio_t led_green = { .port = DB_RGB_LED_PWM_GREEN_PORT, .pin = DB_RGB_LED_PWM_GREEN_PIN };
static const gpio_t led_blue  = { .port = DB_RGB_LED_PWM_BLUE_PORT,  .pin = DB_RGB_LED_PWM_BLUE_PIN  };

int main(void) {
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
