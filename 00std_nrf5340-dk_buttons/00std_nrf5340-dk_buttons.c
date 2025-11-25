#include "nrf5340_network.h"

// per the schematic:
//  - LED1    is connected to P0.28
//  - LED2    is connected to P0.29
//  - LED3    is connected to P0.30
//  - LED4    is connected to P0.31
// all are active LOW.
//
//  - BUTTON1 is connected to P0.23
//  - BUTTON2 is connected to P0.24
//  - BUTTON3 is connected to P0.08
//  - BUTTON4 is connected to P0.09
// all are active LOW.

int main(void) {
    
    // configure LED pins as output
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
    NRF_P0_NS->PIN_CNF[28]        = 0x10000003;            // LED1
    NRF_P0_NS->PIN_CNF[29]        = 0x10000003;            // LED2
    NRF_P0_NS->PIN_CNF[30]        = 0x10000003;            // LED3
    NRF_P0_NS->PIN_CNF[31]        = 0x10000003;            // LED4

    // switch LEDs off
    NRF_P0_NS->OUTSET             = (0x00000001 << 28);    // LED1
    NRF_P0_NS->OUTSET             = (0x00000001 << 29);    // LED2
    NRF_P0_NS->OUTSET             = (0x00000001 << 30);    // LED3
    NRF_P0_NS->OUTSET             = (0x00000001 << 31);    // LED4

    // configure BUTTON pins are input
    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ..AA A: MODE:      1=Event
    // .... .... .... .... ...B BBBB .... .... B: PSEL:      pin
    // .... .... .... .... ..C. .... .... .... C: PORT:      0=P0
    // .... .... .... ..DD .... .... .... .... D: POLARITY:  2=HiToLo
    // .... .... ...E .... .... .... .... .... E: OUTINIT:  no effect in event mode
    // xxxx xxxx xxx0 xx10 xx0? ???? xxxx xx01 
    //    0    0    0    2    0    0    0    1 0x00030001
    NRF_GPIOTE_NS->CONFIG[0]      = 0x00020001 | (23 << 8); // BUTTON1
    NRF_GPIOTE_NS->CONFIG[1]      = 0x00020001 | (24 << 8); // BUTTON2
    NRF_GPIOTE_NS->CONFIG[2]      = 0x00020001 | ( 8 << 8); // BUTTON3
    NRF_GPIOTE_NS->CONFIG[3]      = 0x00020001 | ( 9 << 8); // BUTTON4

    // enable interrupts
    NRF_GPIOTE_NS->INTENSET       = 0x0000000f;
    NVIC_SetPriority(GPIOTE_IRQn,1);
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    NVIC_EnableIRQ(GPIOTE_IRQn);

    // main loop
    while(1) {
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event
    }
}

void GPIOTE_IRQHandler(void) {
    uint8_t pin_state;

    if (NRF_GPIOTE_NS->EVENTS_IN[0] == 0x00000001 ) {
        // BUTTON1 pressed

        // clear
        NRF_GPIOTE_NS->EVENTS_IN[0] = 0x00000000;

        // handle
        NRF_P0_NS->OUTCLR         = (0x00000001 << 28);    // LED1
    }

    if (NRF_GPIOTE_NS->EVENTS_IN[1] == 0x00000001 ) {
        // BUTTON2 pressed

        // clear
        NRF_GPIOTE_NS->EVENTS_IN[1] = 0x00000000;

        // handle
        NRF_P0_NS->OUTCLR         = (0x00000001 << 29);    // LED2
    }

    if (NRF_GPIOTE_NS->EVENTS_IN[2] == 0x00000001 ) {
        // BUTTON2 pressed

        // clear
        NRF_GPIOTE_NS->EVENTS_IN[2] = 0x00000000;

        // handle
        NRF_P0_NS->OUTCLR         = (0x00000001 << 30);    // LED3
    }

    if (NRF_GPIOTE_NS->EVENTS_IN[3] == 0x00000001 ) {
        // BUTTON2 pressed

        // clear
        NRF_GPIOTE_NS->EVENTS_IN[3] = 0x00000000;

        // handle
        NRF_P0_NS->OUTCLR         = (0x00000001 << 31);    // LED4
    }
}
