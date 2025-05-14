#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h" //magistrala komunikacyjna - pozwala rozmawiac z wyswietlaczem
#include "lpc17xx_ssp.h" //modul do komunikacji z urzadzeniami na mikrokontrolerze
#include "lpc17xx_adc.h" //analog to digital converter - do joysticka
#include "lpc17xx_timer.h"  //Funkcje timera - bedziemy ich uzywac
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "joystick.h" //joystick (szok)
#include "oled.h" //wyswietlacz
#include "music.h"

#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26);

static uint32_t notes[] = {
        2272, // A - 440 Hz
        2024, // B - 494 Hz
        3816, // C - 262 Hz
        3401, // D - 294 Hz
        3030, // E - 330 Hz
        2865, // F - 349 Hz
        2551, // G - 392 Hz
        1136, // a - 880 Hz
        1012, // b - 988 Hz
        1912, // c - 523 Hz
        1703, // d - 587 Hz
        1517, // e - 659 Hz
        1432, // f - 698 Hz
        1275, // g - 784 Hz
};

static const char *songs[] = {"E2,E2,E4,E2,E2,E4,E2,G2,C2,D2,E8,F2,F2,F2,F2,F2,E2,E2,E2,E2,D2,D2,E2,D4,G4,E2,E2,E4,E2,E2,E4,E2,G2,C2,D2,E8,F2,F2,F2,F2,F2,E2,E2,E2,G2,G2,F2,D2,C8,",};


static void playNote(uint32_t note, uint32_t durationMs) {

uint32_t t = 0;

if (note > 0) {

 while (t < (durationMs*1000)) {
     NOTE_PIN_HIGH();
     Timer0_us_Wait(note / 2);
     //delay32Us(0, note / 2);

     NOTE_PIN_LOW();
     Timer0_us_Wait(note / 2);
     //delay32Us(0, note / 2);

     t += note;
 }

}
else {
	Timer0_Wait(durationMs);
 //delay32Ms(0, durationMs);
}
}

static uint32_t getNote(uint8_t ch)
{
if (ch >= 'A' && ch <= 'G')
 return notes[ch - 'A'];

if (ch >= 'a' && ch <= 'g')
 return notes[ch - 'a' + 7];

return 0;
}

static uint32_t getDuration(uint8_t ch)
{
if (ch < '0' || ch > '9')
 return 400;

/* number of ms */

return (ch - '0') * 200;
}

static uint32_t getPause(uint8_t ch)
{
switch (ch) {
case '+':
 return 0;
case ',':
 return 5;
case '.':
 return 20;
case '_':
 return 30;
default:
 return 5;
}
}

void playSong(uint8_t *song) {
	uint32_t note = 0;
	uint32_t dur  = 0;
	uint32_t pause = 0;

	/*
	* A song is a collection of tones where each tone is
	* a note, duration and pause, e.g.
	*
	* "E2,F4,"
	*/

	while(*song != '\0') {
	 note = getNote(*song++);
	 if (*song == '\0')
		 break;
	 dur  = getDuration(*song++);
	 if (*song == '\0')
		 break;
	 pause = getPause(*song++);

	 playNote(note, dur);
	 //delay32Ms(0, pause);
	 Timer0_Wait(pause);

}
}
