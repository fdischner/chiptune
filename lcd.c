/* File:    lcd.c
   Author:  Frank Dischner
   Purpose: Contains implementation of all LCD related routines
*/

#include <stdint.h>
#include <util/delay.h>
#include <avr/cpufunc.h>
#include <avr/pgmspace.h>
#include <avr/io.h>

#define LCD_DATA_PORT PORTA
#define LCD_DATA_DDR  DDRA
#define LCD_DATA_PINS PINA
#define LCD_CTRL_PORT PORTC
#define LCD_CTRL_DDR  DDRC
#define LCD_ENABLE    (1 << PC7)
#define LCD_RW        (1 << PC6)
#define LCD_RS        (1 << PC5)

/* LCD_RS values */
#define INSTR 0
#define DATA  1

#define LCD_BUSY_MASK 0x80

static void lcdwrite(uint8_t data, uint8_t rs)
{
    /* set data port as output */
    LCD_DATA_DDR = 0xFF;
    /* write data to port */
    LCD_DATA_PORT = data;

    /* select data/instruction */
    if (rs)
        LCD_CTRL_PORT |= LCD_RS;
    else
        LCD_CTRL_PORT &= ~LCD_RS;

    /* set write command */
    LCD_CTRL_PORT &= ~LCD_RW;
    _NOP();
    /* do write */
    LCD_CTRL_PORT |= LCD_ENABLE;
    _delay_us(0.25);
    LCD_CTRL_PORT &= ~LCD_ENABLE;
    _delay_us(0.25);
    /* pullups off */
    LCD_DATA_PORT = 0x00;
    /* set data port as input */
    LCD_DATA_DDR = 0x00;
}

static uint8_t lcdread(uint8_t rs)
{
    uint8_t data;

    /* set data port as input */
    LCD_DATA_DDR = 0x00;
    /* pullups off */
    LCD_DATA_PORT = 0x00;

    /* select data/instruction */
    if (rs)
        LCD_CTRL_PORT |= LCD_RS;
    else
        LCD_CTRL_PORT &= ~LCD_RS;

    /* set read command */
    LCD_CTRL_PORT |= LCD_RW;
    _NOP();

    /* do read */
    LCD_CTRL_PORT |= LCD_ENABLE;
    _delay_us(0.25);
    data = LCD_DATA_PINS;
    LCD_CTRL_PORT &= ~LCD_ENABLE;
    _delay_us(0.25);

    return data;
}

void lcdbusywait(void)
{
    while (lcdread(INSTR) & LCD_BUSY_MASK);
}

// initialize the LCD
void lcdinit(void)
{
    /* set control pins as outputs */
    LCD_CTRL_DDR |= (LCD_ENABLE | LCD_RW | LCD_RS);
    /* pullups off */
    LCD_DATA_PORT = 0x00;
    /* set data port as input */
    LCD_DATA_DDR = 0x00;

    // switch to 8-bit, two lines
    _delay_ms(17);
    lcdwrite(0x38, INSTR);
    // no, really
    _delay_ms(5);
    lcdwrite(0x38, INSTR);
    // seriously, I mean it this time
    _delay_us(120);
    lcdwrite(0x38, INSTR);

    // the data sheet is not clear on whether the busy flag
    // can be checked yet, so we use a delay to be safe
    _delay_us(120);
    // display on, cursor on, cursor blink
    lcdwrite(0x0C, INSTR);
    // clear display
    lcdbusywait();
    lcdwrite(0x01, INSTR);
    // auto increment on, shift off
    lcdbusywait();
    lcdwrite(0x06, INSTR);
}

void lcdgotoaddr(unsigned char addr)
{
    lcdbusywait();
    lcdwrite(0x80 | addr, INSTR);
}

void lcdgotoxy(unsigned char row, unsigned char column)
{
    unsigned char addr;

    // make sure location is valid
    if (row > 3 || column > 15)
        return;

    addr = column;

    if (row == 1)
        addr += 64;
    else if (row == 2)
        addr += 16;
    else if (row == 3)
        addr += 80;

    lcdgotoaddr(addr);
}

void lcdputch(char cc)
{
    lcdbusywait();
    lcdwrite(cc, DATA);
}

void lcdputstr(char *ss)
{
    unsigned char addr;

    lcdbusywait();
    // seems we need an extra delay before reading the address
    _delay_us(10);
    // get current address
    addr = lcdread(INSTR) & 0x7F;

    // write all characters
    while (*ss)
    {
        lcdputch(*ss++);

        // if we're at the end of a line
        // move to the next one
        if ((++addr & 0x0F) == 0)
        {
            if (addr == 16)
                addr = 64;
            else if (addr == 80)
                addr = 16;
            else if (addr == 32)
                addr = 80;
            else if (addr == 96)
                addr = 0;

            lcdgotoaddr(addr);
        }
    }
}

void lcdputstr_P(uint32_t ss)
{
    unsigned char addr;
    char c;

    lcdbusywait();
    // seems we need an extra delay before reading the address
    _delay_us(30);
    // get current address
    addr = lcdread(INSTR) & 0x7F;

    // write all characters
    while ((c = pgm_read_byte_far(ss++)))
    {
        lcdputch(c);

        // if we're at the end of a line
        // move to the next one
        if ((++addr & 0x0F) == 0)
        {
            if (addr == 16)
                addr = 64;
            else if (addr == 80)
                addr = 16;
            else if (addr == 32)
                addr = 80;
            else if (addr == 96)
                addr = 0;

            lcdgotoaddr(addr);
        }
    }
}

void lcdclear(void)
{
    lcdbusywait();
    lcdwrite(0x01, INSTR);
}

void lcdclearline(unsigned char row)
{
    unsigned char i;

    lcdgotoxy(row, 0);
    for (i = 0; i < 16; i++)
        lcdputch(' ');
}

