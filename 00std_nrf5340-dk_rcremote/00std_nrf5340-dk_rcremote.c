#include <string.h>
#include "nrf5340_network.h"
#include "nrf5340_network_bitfields.h"

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

//=========================== defines =========================================

#define DESTINATION_BROADCAST 'A'

#define PIN_LED1    28
#define PIN_LED2    29
#define PIN_LED3    30
#define PIN_LED4    31

#define PIN_BUTTON1 23
#define PIN_BUTTON2 24
#define PIN_BUTTON3  8
#define PIN_BUTTON4  9

#define CMD_FORWARD 'F'
#define CMD_LEFT    'L'
#define CMD_RIGHT   'R'
#define CMD_STOP    'S'     

//=========================== variables =======================================

static uint8_t pdu[] = {
    0x00, // header
       6, // length
    0x45, 0x53, 0x49, 0x52, 0x4f, 0x49
};

typedef struct {
    uint8_t  cmd;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes =====================================

void leds_set(void);
void leds_all_on(void);

//=========================== main ============================================

int main(void) {
    
    // clear module variables
    memset(&app_vars,0x00,sizeof(app_vars_t));
    app_vars.cmd = CMD_STOP;

    //=== LEDs

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
    NRF_P0_NS->PIN_CNF[PIN_LED1]  = 0x10000003;            // LED1
    NRF_P0_NS->PIN_CNF[PIN_LED2]  = 0x10000003;            // LED2
    NRF_P0_NS->PIN_CNF[PIN_LED3]  = 0x10000003;            // LED3
    NRF_P0_NS->PIN_CNF[PIN_LED4]  = 0x10000003;            // LED4

    // switch LEDs off
    NRF_P0_NS->OUTSET             = (0x00000001 << PIN_LED1);   // LED1
    NRF_P0_NS->OUTSET             = (0x00000001 << PIN_LED2);   // LED2
    NRF_P0_NS->OUTSET             = (0x00000001 << PIN_LED3);   // LED3
    NRF_P0_NS->OUTSET             = (0x00000001 << PIN_LED4);   // LED4
    
    //=== buttons

    // configure BUTTON pins are input
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ...A A: DIR:    0=Input
    // .... .... .... .... .... .... .... ..B. B: INPUT:  0=Connect
    // .... .... .... .... .... .... .... CC.. C: PULL:   3=Pullup
    // .... .... .... .... .... .DDD .... .... D: DRIVE:  0=S0S1
    // .... .... .... ..EE .... .... .... .... E: SENSE:  3=Low
    // .FFF .... .... .... .... .... .... .... F: MCUSEL: 1=NetworkMCU
    // xxxx xxxx xxxx xx00 xxxx xxxx xxxx 1100 
    //    1    0    0    3    0    0    0    c 0x1003000c
    NRF_P0_NS->PIN_CNF[PIN_BUTTON1]        = 0x1003000c; // BUTTON1
    NRF_P0_NS->PIN_CNF[PIN_BUTTON2]        = 0x1003000c; // BUTTON2
    NRF_P0_NS->PIN_CNF[PIN_BUTTON3]        = 0x1003000c; // BUTTON3
    NRF_P0_NS->PIN_CNF[PIN_BUTTON4]        = 0x1003000c; // BUTTON4

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ..AA A: MODE:      1=Event
    // .... .... .... .... ...B BBBB .... .... B: PSEL:      pin
    // .... .... .... .... ..C. .... .... .... C: PORT:      0=P0
    // .... .... .... ..DD .... .... .... .... D: POLARITY:  2=HiToLo
    // .... .... ...E .... .... .... .... .... E: OUTINIT:  no effect in event mode
    // xxxx xxxx xxx0 xx10 xx0? ???? xxxx xx01 
    //    0    0    0    2    0    0    0    1 0x00030001
    NRF_GPIOTE_NS->CONFIG[0]      = 0x00020001 | (PIN_BUTTON1 << 8); // BUTTON1
    NRF_GPIOTE_NS->CONFIG[1]      = 0x00020001 | (PIN_BUTTON2 << 8); // BUTTON2
    NRF_GPIOTE_NS->CONFIG[2]      = 0x00020001 | (PIN_BUTTON3 << 8); // BUTTON3
    NRF_GPIOTE_NS->CONFIG[3]      = 0x00020001 | (PIN_BUTTON4 << 8); // BUTTON4

    // enable interrupts
    NRF_GPIOTE_NS->INTENSET       = 0x0000000f;
    NVIC_SetPriority(GPIOTE_IRQn,1);
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    NVIC_EnableIRQ(GPIOTE_IRQn);
    
    //=== radio

    // configured HF clock
    NRF_CLOCK_NS->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK_NS->EVENTS_HFCLKSTARTED == 0) {}

    // configure radio
    NRF_RADIO_NS->MODE            = (  RADIO_MODE_MODE_Ble_LR125Kbit << RADIO_MODE_MODE_Pos);
    NRF_RADIO_NS->TXPOWER         = (     RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos);
    NRF_RADIO_NS->PCNF0           = (                              8 << RADIO_PCNF0_LFLEN_Pos)          |
                                    (                              1 << RADIO_PCNF0_S0LEN_Pos)          |
                                    (                              0 << RADIO_PCNF0_S1LEN_Pos)          |
                                    (                              2 << RADIO_PCNF0_CILEN_Pos)          |
                                    (     RADIO_PCNF0_PLEN_LongRange << RADIO_PCNF0_PLEN_Pos)           |
                                    (                              3 << RADIO_PCNF0_TERMLEN_Pos);
    NRF_RADIO_NS->PCNF1           = (                    sizeof(pdu) << RADIO_PCNF1_MAXLEN_Pos)         |
                                    (                              0 << RADIO_PCNF1_STATLEN_Pos)        |
                                    (                              3 << RADIO_PCNF1_BALEN_Pos)          |
                                    (      RADIO_PCNF1_ENDIAN_Little << RADIO_PCNF1_ENDIAN_Pos)         |
                                    (   RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos);
    NRF_RADIO_NS->BASE0           = 0xAAAAAAAAUL;
    NRF_RADIO_NS->TXADDRESS       = 0UL;
    NRF_RADIO_NS->RXADDRESSES     = (RADIO_RXADDRESSES_ADDR0_Enabled << RADIO_RXADDRESSES_ADDR0_Pos);
    NRF_RADIO_NS->TIFS            = 1000U;
    NRF_RADIO_NS->CRCCNF          = (         RADIO_CRCCNF_LEN_Three << RADIO_CRCCNF_LEN_Pos)           |
                                    (     RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos);
    NRF_RADIO_NS->CRCINIT         = 0xFFFFUL;
    NRF_RADIO_NS->CRCPOLY         = 0x00065b; // CRC poly: x^16 + x^12^x^5 + 1
    NRF_RADIO_NS->FREQUENCY       = 10;
    NRF_RADIO_NS->PACKETPTR       = (uint32_t)pdu;

    NRF_RADIO_NS->INTENCLR        = 0xffffffff;
    NRF_RADIO_NS->SHORTS          = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
                                    (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
    NRF_RADIO_NS->INTENSET        = (RADIO_INTENSET_DISABLED_Enabled << RADIO_INTENSET_DISABLED_Pos);
    NVIC_EnableIRQ(RADIO_IRQn);

    //=== RTC

    // 1098 7654 3210 9876 5432 1098 7654 3210
    // xxxx xxxx xxxx FEDC xxxx xxxx xxxx xxBA (C=compare 0)
    // 0000 0000 0000 0001 0000 0000 0000 0000 
    //    0    0    0    1    0    0    0    0 0x00010000
    NRF_RTC0_NS->EVTENSET         = 0x00010000;       // enable compare 0 event routing
    NRF_RTC0_NS->INTENSET         = 0x00010000;       // enable compare 0 interrupts

    // enable interrupts
    NVIC_SetPriority(RTC0_IRQn, 1);
    NVIC_ClearPendingIRQ(RTC0_IRQn);
    NVIC_EnableIRQ(RTC0_IRQn);

    // have RTC tick every second
    NRF_RTC0_NS->CC[0]            = 32768>>2;         // (32768>>2)@32kHz = 250ms
    NRF_RTC0_NS->TASKS_START      = 0x00000001;       // start RTC0

    // main loop
    while(1) {
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event
    }
    
    while(1);
}

//=========================== helpers =========================================

void leds_set(void) {

    // all LEDs off
    NRF_P0_NS->OUTSET             = (0x00000001 << PIN_LED1);    // LED1
    NRF_P0_NS->OUTSET             = (0x00000001 << PIN_LED2);    // LED2
    NRF_P0_NS->OUTSET             = (0x00000001 << PIN_LED3);    // LED4
    NRF_P0_NS->OUTSET             = (0x00000001 << PIN_LED4);    // LED3

    // one LED on
    switch(app_vars.cmd) {
        case CMD_FORWARD:
            NRF_P0_NS->OUTCLR     = (0x00000001 << PIN_LED4);    // LED4 on
            break;
        case CMD_LEFT:
            NRF_P0_NS->OUTCLR     = (0x00000001 << PIN_LED2);    // LED2 on
            break;
        case CMD_RIGHT:
            NRF_P0_NS->OUTCLR     = (0x00000001 << PIN_LED3);    // LED3 on
            break;
        case CMD_STOP:
            NRF_P0_NS->OUTCLR     = (0x00000001 << PIN_LED1);    // LED1 on
            break;
    }
}


void leds_all_on(void) {
    NRF_P0_NS->OUTCLR             = (0x00000001 << PIN_LED1);    // LED1
    NRF_P0_NS->OUTCLR             = (0x00000001 << PIN_LED2);    // LED2
    NRF_P0_NS->OUTCLR             = (0x00000001 << PIN_LED3);    // LED4
    NRF_P0_NS->OUTCLR             = (0x00000001 << PIN_LED4);    // LED3
}

//=========================== interrupt handlers ==============================

void RTC0_IRQHandler(void) {

    // handle compare[0]
    if (NRF_RTC0_NS->EVENTS_COMPARE[0] == 0x00000001 ) {

        // clear flag
        NRF_RTC0_NS->EVENTS_COMPARE[0] = 0x00000000;

        // clear COUNTER
        NRF_RTC0_NS->TASKS_CLEAR       = 0x00000001;

        // handle
        leds_all_on();
        pdu[2] = DESTINATION_BROADCAST;
        pdu[3] = app_vars.cmd;
        NRF_RADIO_NS->TASKS_TXEN  = (RADIO_TASKS_TXEN_TASKS_TXEN_Trigger << RADIO_TASKS_TXEN_TASKS_TXEN_Pos);
        while (NRF_RADIO_NS->EVENTS_DISABLED != 0) {}
    }
}

void GPIOTE_IRQHandler(void) {
    uint8_t pin_state;
    
    if (NRF_GPIOTE_NS->EVENTS_IN[0] == 0x00000001 ) {
        // BUTTON1 pressed

        // clear
        NRF_GPIOTE_NS->EVENTS_IN[0] = 0x00000000;

        // handle
        app_vars.cmd              = CMD_STOP;
        leds_set();
    }

    if (NRF_GPIOTE_NS->EVENTS_IN[1] == 0x00000001 ) {
        // BUTTON2 pressed

        // clear
        NRF_GPIOTE_NS->EVENTS_IN[1] = 0x00000000;

        // handle
        app_vars.cmd              = CMD_LEFT;
        leds_set();
    }

    if (NRF_GPIOTE_NS->EVENTS_IN[2] == 0x00000001 ) {
        // BUTTON3 pressed

        // clear
        NRF_GPIOTE_NS->EVENTS_IN[2] = 0x00000000;

        // handle
        app_vars.cmd              = CMD_RIGHT;
        leds_set();
    }

    if (NRF_GPIOTE_NS->EVENTS_IN[3] == 0x00000001 ) {
        // BUTTON4 pressed

        // clear
        NRF_GPIOTE_NS->EVENTS_IN[3] = 0x00000000;

        // handle
        app_vars.cmd              = CMD_FORWARD;
        leds_set();
    }
}

void RADIO_IRQHandler(void) {
    if (NRF_RADIO_NS->EVENTS_DISABLED) {
        NRF_RADIO_NS->EVENTS_DISABLED = 0;
    }
    leds_set();
}
