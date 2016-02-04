/* File:    controller.c
   Author:  Frank Dischner
   Purpose: Contains implementations of all NES controller routines
*/

#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>

/* defines to make code more readable */
#define NES_DDR    DDRD
#define NES_PORT   PORTD
#define NES_PINS   PIND
#define NES_CLK    (1 << PD4)
#define NES_LATCH  (1 << PD5)
#define NES_DATA   (1 << PD6)

void nes_controller_init(void)
{
    /* set clock and latch as outputs */
    NES_DDR |= (NES_CLK | NES_LATCH);
    /* enable pullup on data pin */
    NES_PORT |= NES_DATA;
    /* set initial clock and latch levels */
    NES_PORT &= ~NES_CLK;
    NES_PORT &= ~NES_LATCH;
}

uint8_t nes_controller_read(void)
{
    uint8_t buttons = 0;
    uint8_t i;

    /* latch button state */
    NES_PORT |= NES_LATCH;
    _delay_us(12);
    NES_PORT &= ~NES_LATCH;

    for (i = 0; i < 8; i++)
    {
        /* read input */
        buttons >>= 1;
        if (NES_PINS & NES_DATA)
            buttons |= 0x80;
        _delay_us(6);
        NES_PORT |= NES_CLK;
        _delay_us(6);
        NES_PORT &= ~NES_CLK;
    }

    /* NES buttons are active low */
    buttons = ~buttons;

    return buttons;
}
