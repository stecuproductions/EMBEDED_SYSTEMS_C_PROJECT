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

#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26);
//inits

void init_timer(void){
    TIM_TIMERCFG_Type timerConfig;
    timerConfig.PrescaleOption = TIM_PRESCALE_USVAL;
    timerConfig.PrescaleValue = 1;

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerConfig);
    TIM_Cmd(LPC_TIM0, ENABLE);
}

void init_i2c(void){
    PINSEL_CFG_Type PinCfg;

    /* Configure pins for I2C0 */
    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Pinnum = 27;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin(&PinCfg);

    PinCfg.Pinnum = 28;
    PINSEL_ConfigPin(&PinCfg);

    /* Initialize I2C peripheral */
    I2C_Init(LPC_I2C0, 100000);
    I2C_Cmd(LPC_I2C0, ENABLE);
}
static void init_adc(void)
{
	PINSEL_CFG_Type PinCfg;

	/*
	 * Init ADC pin connect
	 * AD0.0 on P0.23
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 23;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	/* Configuration for ADC :
	 * 	Frequency at 0.2Mhz
	 *  ADC channel 0, no Interrupt
	 */
	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC,ADC_CHANNEL_0,DISABLE);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_0,ENABLE);

}

static void init_ssp(void)
{
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize SPI pin connect
	 * P0.7 - SCK;
	 * P0.8 - MISO
	 * P0.9 - MOSI
	 * P2.2 - SSEL - used as GPIO
	 */
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

	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP1, ENABLE);

}

//calculation functions

const char* dirToString(uint8_t dir){
    switch (dir) {
        case JOYSTICK_UP:    return "UP";
        case JOYSTICK_RIGHT: return "RIGHT";
        case JOYSTICK_DOWN:  return "DOWN";
        case JOYSTICK_LEFT:  return "LEFT";
        default:             return "CENTER";
    }
}

uint8_t generateRandomDirection(void){
    int roll = rand() % 4;
    return (uint8_t[]){JOYSTICK_UP, JOYSTICK_RIGHT,
                       JOYSTICK_DOWN, JOYSTICK_LEFT}[roll];
}

double calculateAverageScore(double *arr) {
    double sum = 0;
    for (int i = 0; i < 5; ++i) sum += arr[i];
    return sum / 5;
}

//music



//Screen display functions

void beginning(void){
    oled_clearScreen(OLED_COLOR_BLACK);
    oled_putString(0, 0,  (uint8_t *)"Przesun joystick", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    oled_putString(0, 10, (uint8_t *)"w PRAWO by zaczac", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

    while (joystick_read() != JOYSTICK_RIGHT) ;
    oled_clearScreen(OLED_COLOR_BLACK);
}

double singleCycle(void) {
    oled_clearScreen(OLED_COLOR_BLACK);

    uint8_t rolledDir = generateRandomDirection();
    oled_putString(10, 10, (uint8_t *)dirToString(rolledDir),
                   OLED_COLOR_WHITE, OLED_COLOR_BLACK);

    clock_t startTime = clock();

    while (joystick_read()!=rolledDir) ;

    clock_t endTime = clock();
    double reactionTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;

    char buf[32];
    oled_clearScreen(OLED_COLOR_BLACK);
    sprintf(buf, "Time: %.3f s", reactionTime);
    oled_putString(10, 10, (uint8_t *)buf, OLED_COLOR_WHITE, OLED_COLOR_BLACK);

    return reactionTime;
}

int main(void){
    init_ssp(); //magistrala kontaktu
    init_i2c(); //tez magistrala kontaktu
    init_timer();
    init_adc();

    joystick_init();
    oled_init();

    srand(time(NULL));
	while(1){
		beginning();


		double scores[5];
		for (int i = 0; i < 5; ++i) {
			scores[i] = singleCycle();
//			for (volatile int j = 0; j < 1000000; ++j) ;
	        Timer0_Wait(5000); //nie wiem czy to zadziala ale buja
		}

		double avg = calculateAverageScore(scores);

		oled_clearScreen(OLED_COLOR_BLACK);
		oled_putString(0, 0,  (uint8_t *)"Game over",  OLED_COLOR_WHITE, OLED_COLOR_BLACK);
		oled_putString(0, 10, (uint8_t *)"Avg score:", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

		char avgBuf[32];
		sprintf(avgBuf, "%.3f s", avg);
		oled_putString(0, 20, (uint8_t *)avgBuf, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
		while(joystick_read()!= (uint8_t)JOYSTICK_UP ){
			continue; //muzyczka

		}
	}
}

