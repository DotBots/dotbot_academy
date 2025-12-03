#include "nrf5340_application.h"

// per the schematic:
//  - LED1    is connected to P0.28
//  - LED2    is connected to P0.29
//  - LED3    is connected to P0.30
//  - LED4    is connected to P0.31
//
//  - BUTTON1 is connected to P0.23
//  - BUTTON2 is connected to P0.24
//  - BUTTON3 is connected to P0.08
//  - BUTTON4 is connected to P0.09

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
    
    // allow the netcore to use the BUTTON pins
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ...A A: DIR:    0=Input
    // .... .... .... .... .... .... .... ..B. B: INPUT:  0=Connect
    // .... .... .... .... .... .... .... CC.. C: PULL:   3=Pullup
    // .... .... .... .... .... .DDD .... .... D: DRIVE:  0=S0S1
    // .... .... .... ..EE .... .... .... .... E: SENSE:  3=Low
    // .FFF .... .... .... .... .... .... .... F: MCUSEL: 1=NetworkMCU
    // x001 xxxx xxxx xx11 xxxx x000 xxxx 1100 
    //    1    0    0    3    0    0    0    c 0x1003000c
    NRF_P0_S->PIN_CNF[23]         = 0x1003000c; // BUTTON1
    NRF_P0_S->PIN_CNF[24]         = 0x1003000c; // BUTTON2
    NRF_P0_S->PIN_CNF[ 8]         = 0x1003000c; // BUTTON3
    NRF_P0_S->PIN_CNF[ 9]         = 0x1003000c; // BUTTON4
    
    // release the netcore's reset line
    NRF_RESET_S->NETWORK.FORCEOFF = 0;
    
    // main loop
    while(1) {
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event
    }
}
