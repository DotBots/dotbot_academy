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
    // .... .... .... .... .... .... .... ...A A: DIR:   1=Output
    // .... .... .... .... .... .... .... ..B. B: INPUT: 1=Disconnect
    // .... .... .... .... .... .... .... CC.. C: PULL:  0=Disabled
    // .... .... .... .... .... .DDD .... .... D: DRIVE: 0=S0S1
    // .... .... .... ..EE .... .... .... .... E: SENSE: 0=Disabled
    // xxxx xxxx xxxx xx00 xxxx xxxx xxxx 0011 
    //    0    0    0    0    0    0    0    3 0x00000003
    NRF_P0_NS->PIN_CNF[22]        = 0x00000003; // red
    NRF_P0_NS->PIN_CNF[23]        = 0x00000003; // green
    NRF_P1_NS->PIN_CNF[ 5]        = 0x00000003; // blue

    // main loop
    while(1) {
        
        // switch LEDs on
        NRF_P0_NS->OUTSET         = (0x00000001 << 22);
        NRF_P0_NS->OUTSET         = (0x00000001 << 23);
        NRF_P1_NS->OUTSET         = (0x00000001 <<  5);

        // switch LEDs off
        NRF_P0_NS->OUTCLR         = (0x00000001 << 22);
        NRF_P0_NS->OUTCLR         = (0x00000001 << 23);
        NRF_P1_NS->OUTCLR         = (0x00000001 <<  5);
    }
}
