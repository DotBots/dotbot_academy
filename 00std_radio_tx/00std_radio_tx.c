#include "nrf5340_network.h"
#include "nrf5340_network_bitfields.h"

static uint8_t pdu[] = {
    0x00, // header
       6, // length
    0x45, 0x53, 0x49, 0x52, 0x4f, 0x49
};

uint32_t numtx;
uint32_t wait;

int main(void) {
    
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

    while(1) {
        
        // send
        NRF_RADIO_NS->TASKS_TXEN  = (RADIO_TASKS_TXEN_TASKS_TXEN_Trigger << RADIO_TASKS_TXEN_TASKS_TXEN_Pos);
        while (NRF_RADIO_NS->EVENTS_DISABLED != 0) {}
        
        // wait a bit
        wait = 0x00ffffff;
        while (wait--);
    }
}

void RADIO_IRQHandler(void) {
    if (NRF_RADIO_NS->EVENTS_DISABLED) {
        NRF_RADIO_NS->EVENTS_DISABLED = 0;
    }
    numtx++;
}
