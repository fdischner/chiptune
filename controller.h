/* File:    controller.h
   Author:  Frank Dischner
   Purpose: Contains prototypes and defines for all NES controller routines
*/

#include <stdint.h>

#ifndef CONTROLLER_H
#define CONTROLLER_H

#define BUTTON_A      0x01
#define BUTTON_B      0x02
#define BUTTON_SELECT 0x04
#define BUTTON_START  0x08
#define BUTTON_UP     0x10
#define BUTTON_DOWN   0x20
#define BUTTON_LEFT   0x40
#define BUTTON_RIGHT  0x80

void nes_controller_init(void);
uint8_t nes_controller_read(void);

#endif /* CONTROLLER_H */
