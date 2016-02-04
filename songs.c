/* File:    songs.c
   Author:  Frank Dischner
   Purpose: Contains implementations for all song related routines as well as
            all song data
*/

#include <stdint.h>
#include <avr/pgmspace.h>

struct song
{
    uint32_t data;
    uint32_t name;
};

#define NUM_SONGS 9

/* include all the song data */

static const prog_uint8_t castlevania_data[] =
{
#include "cv.inc"
};

static const prog_uint8_t castlevania2_data[] =
{
#include "cv2.inc"
};

static const prog_uint8_t ducktales_data[] =
{
#include "dtmoon.inc"
};

static const prog_uint8_t smb1_data[] =
{
#include "smb1.inc"
};

static const prog_uint8_t smb3_data[] =
{
#include "smb3.inc"
};

static const prog_uint8_t tetris1_data[] =
{
#include "tetris1.inc"
};

static const prog_uint8_t tetris2_data[] =
{
#include "tetris2.inc"
};

static const prog_uint8_t tetris3_data[] =
{
#include "tetris3.inc"
};

static const prog_uint8_t zelda_data[] =
{
#include "zelda.inc"
};

static struct song songs[NUM_SONGS];
static uint8_t song_idx = 0;

/* song titles */
static const prog_char smb1_str[] = "Super Mario Bros";
static const prog_char zelda_str[] = "Legend of Zelda Overworld";
static const prog_char castlevania_str[] = "Castlevania     Vampire Killer";
static const prog_char castlevania2_str[] = "Castlevania 2   Bloody Tears";
static const prog_char smb3_str[] = "Super Mario     Bros. 3";
static const prog_char tetris1_str[] = "Tetris          Theme A";
static const prog_char tetris2_str[] = "Tetris          Theme B";
static const prog_char tetris3_str[] = "Tetris          Theme C";
static const prog_char ducktales_str[] = "Ducktales       The Moon";

void songs_init(void)
{
    /* fill the song structs with pointers to the data */
    /* we have to do this in a function because avr-gcc */
    /* only supports 16-bit pointers, so it can only address */
    /* up to 64K. Using the pgm_get_far_address macro gives us */
    /* a pseudo 32-bit pointer, which allows us to access all */
    /* 128K of program memory using the pgm_read_*_far functions */
    songs[0].data = pgm_get_far_address(smb1_data);
    songs[0].name = pgm_get_far_address(smb1_str);
    songs[1].data = pgm_get_far_address(zelda_data);
    songs[1].name = pgm_get_far_address(zelda_str);
    songs[2].data = pgm_get_far_address(castlevania_data);
    songs[2].name = pgm_get_far_address(castlevania_str);
    songs[3].data = pgm_get_far_address(castlevania2_data);
    songs[3].name = pgm_get_far_address(castlevania2_str);
    songs[4].data = pgm_get_far_address(smb3_data);
    songs[4].name = pgm_get_far_address(smb3_str);
    songs[5].data = pgm_get_far_address(tetris1_data);
    songs[5].name = pgm_get_far_address(tetris1_str);
    songs[6].data = pgm_get_far_address(tetris2_data);
    songs[6].name = pgm_get_far_address(tetris2_str);
    songs[7].data = pgm_get_far_address(tetris3_data);
    songs[7].name = pgm_get_far_address(tetris3_str);
    songs[8].data = pgm_get_far_address(ducktales_data);
    songs[8].name = pgm_get_far_address(ducktales_str);
}

uint32_t cur_song_data(void)
{
    return (uint32_t) songs[song_idx].data;
}

uint32_t cur_song_name(void)
{
    return songs[song_idx].name;
}

uint32_t next_song(void)
{
    if (++song_idx >= NUM_SONGS)
        song_idx = 0;

    return cur_song_data();
}

uint32_t prev_song(void)
{
    if (song_idx == 0)
        song_idx = NUM_SONGS - 1;
    else
        song_idx--;

    return cur_song_data();
}
