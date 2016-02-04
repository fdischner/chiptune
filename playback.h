/* File:    playback.h
   Author:  Frank Dischner
   Purpose: Contains prototypes for all audio playback related routines
*/

#include <stdint.h>

#ifndef PLAYBACK_H
#define PLAYBACK_H

enum
{
    PLAYBACK_STATE_STOPPED = 0,
    PLAYBACK_STATE_PLAYING,
    PLAYBACK_STATE_PAUSED
};

void playback_init(void);
void playback_stop(void);
void playback_play(void);
void playback_pause(void);
uint8_t playback_get_state(void);
void playback_set_song(uint32_t addr);
void playback_process_frame(void);
void wait_vblank(void);

#endif /* PLAYBACK_H */
