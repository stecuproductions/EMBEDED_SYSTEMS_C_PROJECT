/*****************************************************************************
 *   This example is controlling the LEDs using the joystick
 *
 *   Copyright(C) 2010, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************/



#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "joystick.h"
#include "oled.h"


uint8_t generateRandomDirection(){
	int roll = rand() % 4 + 1; //random int between 1-4
	switch (roll){
	case 1: return JOYSTICK_UP;
	case 2: return JOYSTICK_RIGHT;
	case 3: return JOYSTICK_DOWN;
	case 4: return JOYSTICK_LEFT;
	}
	return JOYSTICK_CENTER;
}
void beginning(){
    oled_clearScreen(OLED_COLOR_BLACK);
    oled_putString(10, 10, (uint8_t *)"Move joystick to the right to start a new game or <not implemented>.", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

}


double singleCycle(){
    oled_clearScreen(OLED_COLOR_BLACK);
    uint8_t rolledDir = generateRandomDirection();
    clock_t startTime = clock();
    clock_t endTime;
    while(1){
    	uint8_t userDir = joystick_read();
    	if (userDir == rolledDir){
    		endTime = clock();
    		break;
    	}
    }

    double reactionTime = (double)(endTime - startTime); //cos tutaj jest o podzieleniu przez CLOCKS_PER_SEC ALE NIE WIEM, ZOBACZYMY
    char displayInformation[32];
    sprintf(displayInformation, "Time: %.3f s", reactionTime);
    oled_putString(10, 10, (uint8_t *)displayInformation, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    return reactionTime;
}

int main (void) {
    joystick_init();
    oled_init();
    double scores[5];

    beginning();

    for (int i=0; i<5; i++){
    	double cycleScore = singleCycle();
    	scores[i]=cycleScore;
        for (volatile int j = 0; j < 1000000; j++); //SEKUNDA NA ODETCHNIECIE
    }

    oled_clearScreen(OLED_COLOR_BLACK);
    oled_putString(10, 10, (uint8_t *)"Koniec gry!", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

    while(1);
}
