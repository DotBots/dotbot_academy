#include "nrf5340_application.h"

// per the schematic:
//  - the red   LED is connected to P0.22
//  - the green LED is connected to P0.23
//  - the blue  LED is connected to P1.05
// LEDs are active HIGH.
//
// motor control
// P0.07==Vmotor-en: pull LOW to enable H-bridge
// turn motor 1 (left side) clockwise when looking at the wheel from the motor
// - P0.30==AN1 HIGH
// - P0.26==AN2 LOW
// turn motor 2 (left side) clockwise when looking at the wheel from the motor
// - P0.10==BN1 HIGH
// - P0.11==BN2 LOW

#define LED_GREEN       23
#define PIN_VMOTOR_EN    7
#define PIN_AN1         30
#define PIN_AN2         26
#define PIN_BN1         10
#define PIN_BN2         11

int main(void) {
   
    // configure three pins as output
    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ...A A: DIR:    1=Output
    // .... .... .... .... .... .... .... ..B. B: INPUT:  1=Disconnect
    // .... .... .... .... .... .... .... CC.. C: PULL:   0=Disabled
    // .... .... .... .... .... DDDD .... .... D: DRIVE:  0=S0S1
    // .... .... .... ..EE .... .... .... .... E: SENSE:  0=Disabled
    // .FFF .... .... .... .... .... .... .... F: MCUSEL: 0=NetworkMCU
    // x000 xxxx xxxx xx00 xxxx 0000 xxxx 0011 
    //    0    0    0    0    0    0    0    3 0x00000003
    NRF_P0_S->PIN_CNF[LED_GREEN]       = 0x00000003;
    NRF_P0_S->PIN_CNF[PIN_VMOTOR_EN]   = 0x00000003;
    NRF_P0_S->PIN_CNF[PIN_AN1]         = 0x00000003;
    NRF_P0_S->PIN_CNF[PIN_AN2]         = 0x00000003;
    NRF_P0_S->PIN_CNF[PIN_BN1]         = 0x00000003;
    NRF_P0_S->PIN_CNF[PIN_BN2]         = 0x00000003;

    // switch LED_GREEN ON
    NRF_P0_S->OUTSET                   = (0x00000001 << LED_GREEN);
    
    // enable motors (active low)
    NRF_P0_S->OUTCLR                   = (0x00000001 << PIN_VMOTOR_EN);
    
    // full speed forward
    // motor 1 (left) clockwise
    NRF_P0_S->OUTSET                   = (0x00000001 << PIN_AN1); // AN1 HIGH
    NRF_P0_S->OUTCLR                   = (0x00000001 << PIN_AN2); // AN2 LOW
    // motor 2 (right) counter-clockwise
    NRF_P0_S->OUTCLR                   = (0x00000001 << PIN_BN1); // BN1 LOW
    NRF_P0_S->OUTSET                   = (0x00000001 << PIN_BN2); // BN2 HIGH
    
    // main loop
    while(1) {
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event
    }
}
