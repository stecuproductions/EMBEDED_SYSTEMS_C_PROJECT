#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "joystick.h"
#include "oled.h"
#include "eeprom.h"

#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW() GPIO_ClearValue(0, 1<<26);
#define EEPROM_OFFSET 0

// inits

void init_timer(void) {
    TIM_TIMERCFG_Type timerConfig;
    timerConfig.PrescaleOption = TIM_PRESCALE_USVAL;
    timerConfig.PrescaleValue = 1;

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerConfig);
    TIM_Cmd(LPC_TIM0, ENABLE);
}

void init_i2c(void) {
    PINSEL_CFG_Type PinCfg;

    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Pinnum = 27;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin(&PinCfg);

    PinCfg.Pinnum = 28;
    PINSEL_ConfigPin(&PinCfg);

    I2C_Init(LPC_I2C0, 100000);
    I2C_Cmd(LPC_I2C0, ENABLE);
}

static void init_adc(void) {
    PINSEL_CFG_Type PinCfg;

    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Pinnum = 23;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin(&PinCfg);

    ADC_Init(LPC_ADC, 200000);
    ADC_IntConfig(LPC_ADC, ADC_CHANNEL_0, DISABLE);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
}

static void init_ssp(void) {
    SSP_CFG_Type SSP_ConfigStruct;
    PINSEL_CFG_Type PinCfg;

    PinCfg.Funcnum = 2;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 7;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 8;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 9;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Funcnum = 0;
    PinCfg.Portnum = 2;
    PinCfg.Pinnum = 2;
    PINSEL_ConfigPin(&PinCfg);

    SSP_ConfigStructInit(&SSP_ConfigStruct);
    SSP_Init(LPC_SSP1, &SSP_ConfigStruct);
    SSP_Cmd(LPC_SSP1, ENABLE);
}

// calculation functions

const char* dirToString(uint8_t dir) {
    switch (dir) {
        case JOYSTICK_UP: return "UP";
        case JOYSTICK_RIGHT: return "RIGHT";
        case JOYSTICK_DOWN: return "DOWN";
        case JOYSTICK_LEFT: return "LEFT";
        default: return "CENTER";
    }
}

uint8_t generateRandomDirection(void) {
    int roll = rand() % 4;
    return (uint8_t[]){JOYSTICK_UP, JOYSTICK_RIGHT, JOYSTICK_DOWN, JOYSTICK_LEFT}[roll];
}

double calculateAverageScore(double *arr, int size) {
    double sum = 0;
    for (int i = 0; i < 5; ++i) sum += arr[i];
    return sum / size;
}

// music
//Peroid of each note T=1/f. Np: dzwiek A = 440Hz czyli 1/440 = 0.00272727273 czyli w mikrosekundach 2272us
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
//static const char *song = "D4,";
//pioseneczka D - 4s dlugosc polokresu
static const char *song = "D4,B4,B4,A4,A4,G4,E4,D4.D2,E4,E4,A4,F4,D8.D4,d4,d4,c4,c4,B4,G4,E4.E2,F4,F4,A4,A4,G8,";



static void playNote(uint32_t note, uint32_t durationMs) {
    uint32_t t = 0;
    if (note > 0) {
        while (t < (durationMs * 1000)) { //stan on oof * duration
            NOTE_PIN_HIGH(); //buzzer zaczyna grac
            Timer0_us_Wait(note / 2); //polokres
            NOTE_PIN_LOW(); //
            Timer0_us_Wait(note / 2); //polokres
            t += note;
        }
    } else {
        Timer0_Wait(durationMs);
    }
}

static uint32_t getNote(uint8_t ch) {
    if (ch >= 'A' && ch <= 'G') return notes[ch - 'A']; // C - A =>  67  - 65 = 2
    if (ch >= 'a' && ch <= 'g') return notes[ch - 'a' + 7]; //c - a + 7=> 99-97 + 7 = 9
    return 0;
}

static uint32_t getDuration(uint8_t ch) {
    if (ch < '0' || ch > '9') return 400;
    return (ch - '0') * 200; //np: 4 to '4' - '0' = 52 - 48 = 4 * 200 (peeeelny okres)
}

static uint32_t getPause(uint8_t ch) {
    switch (ch) {
        case '+': return 0;
        case ',': return 5;
        case '.': return 20;
        case '_': return 30;
        default: return 5;
    }
}

static void playSong(uint8_t *song) {
    uint32_t note = 0, dur = 0, pause = 0;
    while (*song != '\0') {
        note = getNote(*song++);
        if (*song == '\0') break;
        dur = getDuration(*song++);
        if (*song == '\0') break;
        pause = getPause(*song++);
        playNote(note, dur);
        Timer0_Wait(pause);
    }
}

//read/write best score to eeprom

double readBestScore() {
	double score = 1000.0;
	eeprom_read((uint8_t*)&score, EEPROM_OFFSET, sizeof(score));
	return score;
}

void saveBestScore(double score) {
	eeprom_write((uint8_t*)&score, EEPROM_OFFSET, sizeof(score));
}


int main(void) {
    init_ssp();
    init_i2c();
    init_timer();
    init_adc();
    eeprom_init();

    GPIO_SetDir(2, 1<<0, 1);
    GPIO_SetDir(2, 1<<1, 1);
    GPIO_SetDir(0, 1<<27, 1);
    GPIO_SetDir(0, 1<<28, 1);
    GPIO_SetDir(2, 1<<13, 1);
    GPIO_SetDir(0, 1<<26, 1);

    GPIO_ClearValue(0, 1<<27);
    GPIO_ClearValue(0, 1<<28);
    GPIO_ClearValue(2, 1<<13);

    joystick_init();
    oled_init();
    Timer0_Wait(1000);

    double bestScore = readBestScore();

    typedef enum {
        GAME_WAIT_START,
        GAME_SHOW_DIR,
        GAME_WAIT_MOVE,
        GAME_SHOW_RESULT
    } GameState;

    GameState gameState = GAME_WAIT_START;
    int cycles = 5;
    double scores[cycles];
    int iterator = 0;
    uint8_t rolledDir;
    double reactionTime;
    double avg;
    clock_t startTime, endTime;
    while (1) {
        switch (gameState) {
            case GAME_WAIT_START:
            	oled_clearScreen(OLED_COLOR_BLACK);
            	oled_putString(0, 0, (uint8_t *)"Move joystick:", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            	oled_putString(0, 10, (uint8_t *)"RIGHT - start game", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            	oled_putString(0,20, (uint8_t *)"LEFT - show best score", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            	if (joystick_read() == JOYSTICK_RIGHT) {
            		oled_clearScreen(OLED_COLOR_BLACK);
            		gameState = GAME_SHOW_DIR;
                }
            	else if (joystick_read() == JOYSTICK_LEFT) {
            		oled_clearScreen(OLED_COLOR_BLACK);
                	oled_putString(0, 0, (uint8_t *)"BEST SCORE:", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                	if(bestScore == 1000.0) {
                    	oled_putString(0, 10, (uint8_t *)"No best score yet", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                	}
                	else {
                		char b[32];
                		sprintf(b, "%.3f s", bestScore);
                    	oled_putString(0, 10, (uint8_t *)b, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                	}
                	Timer0_Wait(3000);
                	gameState = GAME_WAIT_START;
            	}
                break;

            case GAME_SHOW_DIR:
                oled_clearScreen(OLED_COLOR_BLACK);
                rolledDir = generateRandomDirection();
                oled_putString(10, 10, (uint8_t *)dirToString(rolledDir), OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                startTime = clock();
                Timer0_Wait(3000);
                gameState = GAME_WAIT_MOVE;
                break;

            case GAME_WAIT_MOVE:
                if (joystick_read() == rolledDir) {
                    endTime = clock();
                    reactionTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
                    scores[iterator++] = reactionTime;
                    oled_clearScreen(OLED_COLOR_BLACK);
                    char buf[32];
                    sprintf(buf, "Time: %.3f s", reactionTime);
                    oled_putString(10, 10, (uint8_t *)buf, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                    gameState = (iterator < cycles) ? GAME_SHOW_DIR : GAME_SHOW_RESULT;
                }
                break;

            case GAME_SHOW_RESULT:
                avg = calculateAverageScore(scores, cycles);
                oled_clearScreen(OLED_COLOR_BLACK);
                oled_putString(0, 0, (uint8_t *)"Game over", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                oled_putString(0, 10, (uint8_t *)"Avg score:", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                char avgBuf[32];
                sprintf(avgBuf, "%.3f s", avg);
                oled_putString(0, 20, (uint8_t *)avgBuf, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                if(avg < bestScore) {
                	bestScore = avg;
                    oled_putString(0, 30, (uint8_t *)"NEW BEST SCORE!", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                    saveBestScore(bestScore);
                }
                playSong((uint8_t*) song);
                Timer0_Wait(1000);
                iterator = 0;
                gameState = GAME_WAIT_START;
                break;
        }
    }
}