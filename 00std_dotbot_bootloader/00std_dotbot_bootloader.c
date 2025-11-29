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

#define PIN_VMOTOR_EN    7
#define PIN_AN1         30
#define PIN_AN2         26
#define PIN_BN1         10
#define PIN_BN2         11

int main(void) {
   
    // allow the netcore to use the LED pins
    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ...A A: DIR:    1=Output
    // .... .... .... .... .... .... .... ..B. B: INPUT:  1=Disconnect
    // .... .... .... .... .... .... .... CC.. C: PULL:   0=Disabled
    // .... .... .... .... .... DDDD .... .... D: DRIVE:  0=S0S1
    // .... .... .... ..EE .... .... .... .... E: SENSE:  0=Disabled
    // .FFF .... .... .... .... .... .... .... F: MCUSEL: 1=NetworkMCU
    // x001 xxxx xxxx xx00 xxxx 0000 xxxx 0011 
    //    1    0    0    0    0    0    0    3 0x10000003
    NRF_P0_S->PIN_CNF[22]              = 0x10000003; // red
    NRF_P0_S->PIN_CNF[23]              = 0x10000003; // green
    NRF_P1_S->PIN_CNF[ 5]              = 0x10000003; // blue

    // allow the netcore to use the motor pins
    NRF_P0_S->PIN_CNF[PIN_VMOTOR_EN]   = 0x10000003; // red
    NRF_P0_S->PIN_CNF[PIN_AN1]         = 0x10000003; // red
    NRF_P0_S->PIN_CNF[PIN_AN2]         = 0x10000003; // red
    NRF_P0_S->PIN_CNF[PIN_BN1]         = 0x10000003; // red
    NRF_P0_S->PIN_CNF[PIN_BN2]         = 0x10000003; // red

    // release the netcore's reset line
    NRF_RESET_S->NETWORK.FORCEOFF = 0;

    // main loop
    while(1) {
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event
    }
}
