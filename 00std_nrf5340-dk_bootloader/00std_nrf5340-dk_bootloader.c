#include "nrf5340_application.h"

// per the schematic:
//  - the LED1 is connected to P0.28
//  - the LED2 is connected to P0.29
//  - the LED3 is connected to P0.30
//  - the LED4 is connected to P0.31

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
    NRF_P0_S->PIN_CNF[28]         = 0x10000003; // LED1
    NRF_P0_S->PIN_CNF[29]         = 0x10000003; // LED2
    NRF_P0_S->PIN_CNF[30]         = 0x10000003; // LED3
    NRF_P0_S->PIN_CNF[31]         = 0x10000003; // LED4

    // release the netcore's reset line
    NRF_RESET_S->NETWORK.FORCEOFF = 0;

    // main loop
    while(1) {
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event
    }
}
