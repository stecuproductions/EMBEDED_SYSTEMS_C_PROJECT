

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


double calculateAverageScore(double* arr){
	int avg=0;
	for(int i=0; i<5; i++){
		avg+=arr[i];
	}
	return avg/5;
}

void beginning(){
    oled_clearScreen(OLED_COLOR_BLACK);
    oled_putString(10, 10, (uint8_t *)"Move joystick to the right", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    oled_putString(10, 20, (uint8_t *)"to start a new game or...", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    while(1){
    	uint8_t joy = joystick_read();
    	if (joy == JOYSTICK_UP){
    		break;
    	}
    }
    oled_clearScreen(OLED_COLOR_BLACK);
}



double singleCycle(){
    oled_clearScreen(OLED_COLOR_BLACK);
    uint8_t rolledDir = generateRandomDirection();
    clock_t startTime = clock();
    oled_putString(10, 10, (uint8_t *)rolledDir, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    clock_t endTime;
    while(1){
    	uint8_t userDir = joystick_read();
    	if (userDir == rolledDir){
    		endTime = clock();
    		break;
    	}
    }

    double reactionTime = (double)(endTime - startTime) / CLOCKS_PER_SEC; //cos tutaj jest o podzieleniu przez CLOCKS_PER_SEC ALE NIE WIEM, ZOBACZYMY
    char displayInformation[32];
    oled_clearScreen(OLED_COLOR_BLACK);
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
    double averageScore = calculateAverageScore(scores);
    oled_clearScreen(OLED_COLOR_BLACK);
    oled_putString(0, 0, (uint8_t *)"Game over", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    oled_putString(0, 10, (uint8_t *)"avg score:", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    char avgScoreText[32];
    sprintf(avgScoreText, "%.3f s", averageScore);
    oled_putString(0, 20, (uint8_t *)avgScoreText, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    while(1);
}
