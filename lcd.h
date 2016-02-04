/* File:    lcd.h
   Author:  Frank Dischner
   Purpose: Contains prototypes for all LCD related routines
*/

#include <avr/pgmspace.h>

#ifndef _LCD_H_
#define _LCD_H_

void lcdinit(void);
void lcdbusywait(void);
void lcdgotoaddr(unsigned char addr);
void lcdgotoxy(unsigned char row, unsigned char column);
void lcdputch(char cc);
void lcdputstr(char *ss);
void lcdputstr_P(uint32_t ss);
void lcdclear(void);
void lcdclearline(unsigned char row);

#endif /* _LCD_H_ */
