/* File:    main.c
   Author:  Frank Dischner
   Purpose: Contains the main function, which initializes the hardware and
            then starts the main loop to process audio data and user input
*/

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "playback.h"
#include "songs.h"
#include "controller.h"
#include "lcd.h"

int main(void)
{
    /* enable all pullups to prevent floating inputs */
    PORTA = 0xFF;
    PORTB = 0xFF;
    PORTC = 0xFF;
    PORTD = 0xFF;

    /* set PB0 as output for timing */
    DDRB |= (1 << DDB0);
    PORTB &= ~(1 << PB0);

    /* initialize LCD */
    lcdinit();
    /* initialize NES controller */
    nes_controller_init();
    /* initialize playback engine */
    playback_init();
    /* initialize song data */
    songs_init();

    /* set initial song */
    playback_set_song(cur_song_data());
    /* display song name */
    lcdputstr_P(cur_song_name());
    /* display playback status */
    lcdgotoxy(3, 0);
    lcdputstr_P((uint32_t) PSTR("Stopped"));

    /* enable interrupts to get playback going */
    sei();

    /* main loop */
    while (1)
    {
        static uint8_t prev_buttons = 0;
        uint8_t buttons;
        uint8_t changed;

        /* set pin high so we can time the loop */
        /* this allows us to measure how long
           it takes to process each frame */
        PORTB |= (1 << PB0);

        /* process one frame */
        playback_process_frame();

        /* process user input */

        /* get current button state */
        buttons = nes_controller_read();
        /* check which buttons changed state */
        changed = buttons ^ prev_buttons;
        /* save current button state */
        prev_buttons = buttons;
        /* check which buttons were pressed */
        buttons = changed & prev_buttons;
        if (buttons & BUTTON_START)
        {
            /* clear status line */
            lcdclearline(3);
            /* update status */
            if (playback_get_state() != PLAYBACK_STATE_PLAYING)
            {
                /* start playback */
                playback_play();
                lcdgotoxy(3, 0);
                lcdputstr_P((uint32_t) PSTR("Playing"));
            }
            else
            {
                /* pause playback */
                playback_pause();
                lcdgotoxy(3, 0);
                lcdputstr_P((uint32_t) PSTR("Paused"));
            }
        }
        else if (buttons & BUTTON_SELECT)
        {
            /* stop playback */
            playback_stop();
            /* clear status line */
            lcdclearline(3);
            /* update status */
            lcdgotoxy(3, 0);
            lcdputstr_P((uint32_t) PSTR("Stopped"));
        }
        else if (buttons & BUTTON_LEFT)
        {
            uint8_t state;

            /* save playback state */
            state = playback_get_state();
            /* stop playback */
            playback_stop();
            /* set prev song */
            playback_set_song(prev_song());
            lcdclearline(0);
            lcdclearline(1);
            lcdgotoaddr(0);
            lcdputstr_P(cur_song_name());
            /* restart playback */
            if (state == PLAYBACK_STATE_PLAYING)
            {
                playback_play();
            }
            else
            {
                /* make sure status is stopped */
                /* clear status line */
                lcdclearline(3);
                /* update status */
                lcdgotoxy(3, 0);
                lcdputstr_P((uint32_t) PSTR("Stopped"));
            }
        }
        else if (buttons & BUTTON_RIGHT)
        {
            uint8_t state;

            /* save playback state */
            state = playback_get_state();
            /* stop playback */
            playback_stop();
            /* set next song */
            playback_set_song(next_song());
            lcdclearline(0);
            lcdclearline(1);
            lcdgotoaddr(0);
            lcdputstr_P(cur_song_name());
            /* restart playback */
            if (state == PLAYBACK_STATE_PLAYING)
            {
                playback_play();
            }
            else
            {
                /* make sure status is stopped */
                /* clear status line */
                lcdclearline(3);
                /* update status */
                lcdgotoxy(3, 0);
                lcdputstr_P((uint32_t) PSTR("Stopped"));
            }
        }

        /* set pin low to end loop timing */
        PORTB &= ~(1 << PB0);

        /* wait for next frame start */
        wait_vblank();
    }
}
