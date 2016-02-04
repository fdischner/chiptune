/* File:    playback.c
   Author:  Frank Dischner
   Purpose: Contains implementations for all audio playback related routines
*/

#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "playback.h"

/* 40kHz / 60 fps */
#define SAMPLES_PER_FRAME 667

/* vblank indicates that output buffers have been swapped */
static volatile uint8_t vblank = 0;
/* index of currently playing output buffer */
static volatile uint8_t out_idx = 0;
/* state needed for wave generation */
static uint16_t step[4] = { 0, 0, 0, 0 };
static int8_t volume[4] = { 0, 0, 0, 0 };
static uint8_t duty[2] = { 0x80, 0x80 };
static uint16_t phase[4] = { 0, 0, 0, 0 };
static uint16_t lfsr = 1;
static uint8_t lfsr_mode = 0;
/* double buffered output */
static uint8_t outbuf[2][SAMPLES_PER_FRAME];
/* current playback state */
static uint8_t state = PLAYBACK_STATE_STOPPED;
/* 'pointers' to song info */
static uint32_t song_start = 0;
static uint32_t song_repeat = 0;
static uint32_t song_pos = 0;
/* current frame */
static uint16_t frame = 0;
/* last frame containing an event */
static uint16_t last_frame = 0;

/* interrupt routine used to output samples */
ISR(TIMER1_COMPA_vect)
{
    static unsigned int count = 0;
    static uint8_t *p = outbuf[0];

    /* output sample */
    OCR0A = *p++;

    /* clear vblank if it was set last time */
    vblank = 0;

    /* increment sample counter */
    if (++count == SAMPLES_PER_FRAME)
    {
        /* swap output buffer */
        out_idx = 1 - out_idx;
        p = outbuf[out_idx];
        /* indicate that buffer swap has occurred */
        vblank = 1;
        /* reset sample counter */
        count = 0;
    }
}

static void calculate_frame(uint8_t *buf)
{
    int i;

    /* calculate all samples in frame */
    for (i = 0; i < SAMPLES_PER_FRAME; i++)
    {
        int8_t tmp1, tmp2;

        /* Parts of this algorithm, namely the square and triangle waves, */
        /* were inspired by the assembly code in Craft by Linus Akesson */
        /* http://www.linusakesson.net/scene/craft/index.php */

        /* first square wave */
        phase[0] += step[0];
        tmp1 = volume[0];
        if (((phase[0] >> 8) & 0xE0) >= duty[0])
        {
            tmp1 = -tmp1;
        }

        /* second square wave */
        phase[1] += step[1];
        if (((phase[1] >> 8) & 0xE0) >= duty[1])
        {
            tmp1 -= volume[1];
        }
        else
        {
            tmp1 += volume[1];
        }

        /* triangle wave */
        /* to prevent pops in the output, caused by discontinuities,   */
        /* the triangle output is always 'on', but stays at a constant */
        /* value when not playing. This adds a DC offset, but its much */
        /* simpler than trying to filter it and the NES does the same. */
        if (volume[2])
            phase[2] += step[2];

        /* top 7 bits of the phase are the output level */
        /* the upper half of the phase range is negated */
        /* to give output values 0 -> 64 -> 0 (triangle) */
        /* instead of 0 -> 127 (sawtooth) */
        tmp2 = (((int16_t) phase[2]) >> 9);
        if (tmp2 & 0x80)
            tmp2 = -tmp2;
        tmp1 += tmp2;

        /* noise */
        /* only increment the lfsr if channel is on */
        if (volume[3])
            phase[3] += step[3];

        /* lsb determines output value */
        tmp2 = (lfsr & 0x1) ? -volume[3] : volume[3];
        tmp1 += tmp2;
        /* clock lfsr */
        if (phase[3] & 0x8000)
        {
            uint8_t tap1, tap2;

            /* first tap is always lsb */
            tap1 = lfsr & 0x1;
            /* second tap depends on mode (short/long) */
            if (lfsr_mode)
                tap2 = (lfsr & (1 << 6)) ? 1 : 0;
            else
                tap2 = (lfsr & (1 << 1)) ? 1 : 0;

            /* shift right */
            lfsr >>= 1;
            /* load bit 14 to give a 15 bit lfsr */
            if (tap1 ^ tap2)
                lfsr |= (1 << 14);

            /* decrement phase counter */
            phase[3] ^= 0x8000;
        }

        /* normalize range to 0-255 */
        /* 128 for DC offset and 32 for triangle offset */
        *buf++ = (uint8_t) (tmp1 + 128 - 32);
    }
}

void playback_init(void)
{
    /* initialize buffers with silence */
    memset(outbuf, 0x80, 2 * SAMPLES_PER_FRAME);

    /* set PB3 (OC0A) as output */
    DDRB |= (1 << PB3);

    /* set fast PWM and compare output A non-inverting */
    TCCR0A = 0x83;
    /* stop timer */
    TCCR0B = 0x00;
    /* reset counter */
    TCNT0 = 0x00;
    /* zero (signed) output */
    OCR0A = 0x80;
    /* start clock, no prescaler */
    TCCR0B = 0x01;

    /* stop timer and set reset on OCR1A match */
    TCCR1A = 0x00;
    TCCR1B = 0x08;
    /* match when count reaches 500 (40KHz with 20MHz clock) */
    OCR1AH = 0x01;
    OCR1AL = 0xF4;
    /* reset counter */
    TCNT1H = 0x00;
    TCNT1L = 0x00;
    /* enable match interrupt */
    TIMSK1 = 0x02;
    /* start timer with no prescaler */
    TCCR1B |= 0x01;
}

void playback_stop(void)
{
    state = PLAYBACK_STATE_STOPPED;

    /* reset output state */
    step[0] = step[1] = step[2] = step[3] = 0;
    volume[0] = volume[1] = volume[2] = volume[3] = 0;
    duty[0] = duty[1] = 0x80;
    phase[0] = phase[1] = phase[2] = phase[3] = 0;
    lfsr = 1;
    lfsr_mode = 0;
    frame = last_frame = 0;

    /* reset song to beginning */
    song_pos = song_repeat = song_start;
}

void playback_play(void)
{
    state = PLAYBACK_STATE_PLAYING;
}

void playback_pause(void)
{
    state = PLAYBACK_STATE_PAUSED;
}

uint8_t playback_get_state(void)
{
    return state;
}

void playback_set_song(uint32_t addr)
{
    song_start = song_repeat = song_pos = addr;
}

void playback_process_frame(void)
{
    uint8_t *out;

    /* write to buffer that isn't currently being played */
    /* NOTE: though out_idx is modified in the isr, single byte */
    /* accesses are inherently atomic so this is safe */
    /* additionally, this function should only ever be called */
    /* immediately after out_idx is modified (after buffer swap) */
    out = outbuf[1 - out_idx];

    /* if not playing, output silence */
    if (state != PLAYBACK_STATE_PLAYING || !song_start || !song_pos)
    {
        memset(out, 0x80, SAMPLES_PER_FRAME);
        return;
    }

    /* process all events for this frame */
    while ((last_frame + pgm_read_byte_far(song_pos)) == frame)
    {
        uint8_t command, channel;

        song_pos++;
        command = pgm_read_byte_far(song_pos++);
        channel = command & 0x0F;
        switch (command & 0xF0)
        {
            /* step (frequency) */
            case 0x00:
                step[channel] = pgm_read_byte_far(song_pos++);
                step[channel] |= pgm_read_byte_far(song_pos++) << 8;
                break;
            /* volume */
            case 0x10:
                volume[channel] = pgm_read_byte_far(song_pos++);
                break;
            /* duty cycle (square wave only) */
            case 0x30:
                duty[channel] = pgm_read_byte_far(song_pos++);
                break;
            /* noise channel mode */
            case 0x40:
                lfsr_mode = pgm_read_byte_far(song_pos++);
                break;
            /* set repeat point */
            case 0xE0:
                song_repeat = song_pos;
                break;
            /* jump to repeat point */
            case 0xF0:
                song_pos = song_repeat;
                break;
            default:
                break;
        }

        last_frame = frame;
    }

    /* calculate this frame */
    calculate_frame(out);

    /* increment frame count */
    frame++;
}

void wait_vblank(void)
{
    /* wait for next frame (buffer swap) */
    while (!vblank);
}
