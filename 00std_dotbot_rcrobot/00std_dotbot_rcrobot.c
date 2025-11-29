#include <stdio.h>
#include "nrf5340_network.h"
#include "nrf5340_network_bitfields.h"

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

#define CMD_FORWARD     'F'
#define CMD_LEFT        'L'
#define CMD_RIGHT       'R'
#define CMD_STOP        'S'  

static uint8_t pdu[8+1] = { 0 };

uint32_t numrx;

int main(void) {
    
    //=== motor

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
    NRF_P0_NS->PIN_CNF[PIN_VMOTOR_EN]  = 0x00000003;
    NRF_P0_NS->PIN_CNF[PIN_AN1]        = 0x00000003;
    NRF_P0_NS->PIN_CNF[PIN_AN2]        = 0x00000003;
    NRF_P0_NS->PIN_CNF[PIN_BN1]        = 0x00000003;
    NRF_P0_NS->PIN_CNF[PIN_BN2]        = 0x00000003;

    // enable motors (active low)
    NRF_P0_NS->OUTCLR                  = (0x00000001 << PIN_VMOTOR_EN);

    //=== radio

    // confiureg HF clock
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
    NRF_RADIO_NS->TIFS            = 0;
    NRF_RADIO_NS->CRCCNF          = (         RADIO_CRCCNF_LEN_Three << RADIO_CRCCNF_LEN_Pos)           |
                                    (     RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos);
    NRF_RADIO_NS->CRCINIT         = 0xFFFFUL;
    NRF_RADIO_NS->CRCPOLY         = 0x00065b; // CRC poly: x^16 + x^12^x^5 + 1
    NRF_RADIO_NS->FREQUENCY       = 10;
    NRF_RADIO_NS->PACKETPTR       = (uint32_t)pdu;

    // receive
    NRF_RADIO_NS->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
                        (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos) |
                        (RADIO_SHORTS_DISABLED_RXEN_Enabled << RADIO_SHORTS_DISABLED_RXEN_Pos);
    NRF_RADIO_NS->TASKS_RXEN    = 1;

    NRF_RADIO_NS->INTENCLR = 0xffffffff;
    NVIC_EnableIRQ(RADIO_IRQn);
    NRF_RADIO_NS->INTENSET = (RADIO_INTENSET_DISABLED_Enabled << RADIO_INTENSET_DISABLED_Pos);

    while(1) {
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event
    }
}

void RADIO_IRQHandler(void) {
    if (NRF_RADIO_NS->EVENTS_DISABLED) {
        NRF_RADIO_NS->EVENTS_DISABLED = 0;

        if (NRF_RADIO_NS->CRCSTATUS != RADIO_CRCSTATUS_CRCSTATUS_CRCOk) {
            puts("Invalid CRC");
        } else {
            printf("Received packet (%dB): %s\n", pdu[1], &pdu[2]);
            switch(pdu[3]) {
                case CMD_FORWARD:
                    // motor 1 (left) clockwise
                    NRF_P0_NS->OUTSET                  = (0x00000001 << PIN_AN1); // AN1 HIGH
                    NRF_P0_NS->OUTCLR                  = (0x00000001 << PIN_AN2); // AN2 LOW
                    // motor 2 (right) counter-clockwise
                    NRF_P0_NS->OUTCLR                  = (0x00000001 << PIN_BN1); // BN1 LOW
                    NRF_P0_NS->OUTSET                  = (0x00000001 << PIN_BN2); // BN2 HIGH
                    break;
                case CMD_LEFT:
                    // motor 1 (left)  counter-clockwise
                    NRF_P0_NS->OUTCLR                  = (0x00000001 << PIN_AN1); // AN1 LOW
                    NRF_P0_NS->OUTSET                  = (0x00000001 << PIN_AN2); // AN2 HIGH
                    // motor 2 (right) counter-clockwise
                    NRF_P0_NS->OUTCLR                  = (0x00000001 << PIN_BN1); // BN1 LOW
                    NRF_P0_NS->OUTSET                  = (0x00000001 << PIN_BN2); // BN2 HIGH
                    break;
                case CMD_RIGHT:
                    // motor 1 (left)  clockwise
                    NRF_P0_NS->OUTSET                  = (0x00000001 << PIN_AN1); // AN1 HIGH
                    NRF_P0_NS->OUTCLR                  = (0x00000001 << PIN_AN2); // AN2 LOW
                    // motor 2 (right) clockwise
                    NRF_P0_NS->OUTSET                  = (0x00000001 << PIN_BN1); // BN1 HIGH
                    NRF_P0_NS->OUTCLR                  = (0x00000001 << PIN_BN2); // BN2 LOW
                    break;
                default:
                    // motor 1 (left)  stopped
                    NRF_P0_NS->OUTCLR                  = (0x00000001 << PIN_AN1); // AN1 LOW
                    NRF_P0_NS->OUTCLR                  = (0x00000001 << PIN_AN2); // AN2 LOW
                    // motor 2 (right) stopped
                    NRF_P0_NS->OUTCLR                  = (0x00000001 << PIN_BN1); // BN1 LOW
                    NRF_P0_NS->OUTCLR                  = (0x00000001 << PIN_BN2); // BN2 LOW
                    break;
            }
        }
    }
}
