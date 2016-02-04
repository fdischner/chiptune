/* File:    songs.h
   Author:  Frank Dischner
   Purpose: Contains prototypes for all song related routines
*/

#include <stdint.h>
#include <avr/pgmspace.h>

#ifndef SONG_H
#define SONG_H

void songs_init(void);
uint32_t cur_song_data(void);
uint32_t cur_song_name(void);
uint32_t next_song(void);
uint32_t prev_song(void);

#endif /* SONG_H */
