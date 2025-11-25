#include "nrf5340_network.h"

// per the schematic:
//  - the red   LED is connected to P0.22
//  - the green LED is connected to P0.23
//  - the blue  LED is connected to P1.05
// all are active HIGH.

int main(void) {
   
    // configure three pins as output
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
    NRF_P0_NS->PIN_CNF[22]        = 0x10000003;            // red
    NRF_P0_NS->PIN_CNF[23]        = 0x10000003;            // green
    NRF_P1_NS->PIN_CNF[ 5]        = 0x10000003;            // blue

    // main loop
    while(1) {
        
        // switch LEDs on
        NRF_P0_NS->OUTSET         = (0x00000001 << 22);    // red
        NRF_P0_NS->OUTSET         = (0x00000001 << 23);    // green
        NRF_P1_NS->OUTSET         = (0x00000001 <<  5);    // blue

        // switch LEDs off
        NRF_P0_NS->OUTCLR         = (0x00000001 << 22);    // red
        NRF_P0_NS->OUTCLR         = (0x00000001 << 23);    // green
        NRF_P1_NS->OUTCLR         = (0x00000001 <<  5);    // blue
    }
}
