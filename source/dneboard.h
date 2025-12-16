/***************************************************************************//**
@file     dneboard.h
@brief    Display and encoder board management
@author   Emma Herrera
******************************************************************************/

#ifndef _DNEBOARD_H_
#define _DNEBOARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** DNE BOARD defines **********************************************************/

#define RCHA          	PORTNUM2PIN(PB,19) // PTB19
#define RCHB          	PORTNUM2PIN(PB,18) // PTB18
#define R_SWITCH       	PORTNUM2PIN(PA,2) // PTA2

#define STATUS0        	PORTNUM2PIN(PB,23) // PTB23
#define STATUS1        	PORTNUM2PIN(PA,1)  // PTA1

#define SEGA          	PORTNUM2PIN(PC,5) // PTC5
#define SEGB          	PORTNUM2PIN(PC,7) // PTC7
#define SEGC          	PORTNUM2PIN(PC,0) // PTC0
#define SEGD          	PORTNUM2PIN(PC,9) // PTC9
#define SEGE          	PORTNUM2PIN(PC,8) // PTC8
#define SEGF          	PORTNUM2PIN(PC,1) // PTC1
#define SEGG          	PORTNUM2PIN(PC,3) // PTBC3
#define SEGDP           PORTNUM2PIN(PC,2) // PTC2

#define SEL0          	PORTNUM2PIN(PD,0) // PTD0
#define SEL1          	PORTNUM2PIN(PD,2) // PTD2

#define PORT_DATA       PORTNUM2PIN(PB,10)
#define PORT_CLOCK      PORTNUM2PIN(PB,3)
#define PORT_ENABLE     PORTNUM2PIN(PB,2)

//FRDM Leds

#define PIN_LED_RED     PORTNUM2PIN(PB,22) // PTB22
#define PIN_LED_GREEN   PORTNUM2PIN(PE,26) // PTE26
#define PIN_LED_BLUE    PORTNUM2PIN(PB,21) // PTB21

/*******************************************************************************
 ******************************************************************************/

#endif // _DNEBOARD_H_
