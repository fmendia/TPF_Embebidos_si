/*******************************************************************************
    @file     leds.c
    @brief    Leds functions
    @author   Grupo 1
******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "leds.h"
#include "dneboard.h"
#include "gpio.h"
#include <stdbool.h>

#define LED_SLEEP_TIME 20 //in ms

OS_TCB  LedsTaskTCB;
CPU_STK LedsTaskStk[LEDS_TASK_STACKSIZE];

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void Led_Set(uint8_t led);

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

bool leds[3] = {0}; // Array to hold the state of each LED [0:d3, 1:d2, 2:d1]

bool color_leds = {0}; // Array to hold the state of each COLOR LED [0:R, 1:G, 2:B]

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
*******************************************************************************
******************************************************************************/

void Leds_Init(void) {
    gpioMode(STATUS0, OUTPUT);
    gpioMode(STATUS1, OUTPUT);

    gpioMode(PIN_LED_RED, OUTPUT);
    gpioMode(PIN_LED_BLUE, OUTPUT);
    gpioMode(PIN_LED_GREEN, OUTPUT);
    gpioWrite(PIN_LED_RED, HIGH);
    gpioWrite(PIN_LED_BLUE, HIGH);
    gpioWrite(PIN_LED_GREEN, HIGH);
//    SysTick_Manager(Led_Callback, 25); // Con 20 se ve re brillante
}

void Leds_Task(void *p_arg){
    (void)p_arg;
    OS_ERR err;

    while (1) {
        // Update LEDs based on their states
        for (uint8_t led = 0; led < 3; led++) {
            if (leds[led]) {
                Led_Set(led);
                break; // Only one LED can be on at a time
            }
            if (led == 2) {
                Led_Set(3); // Turn off all LEDs if none are on
            }
        }

        // Sleep until next update
        OSTimeDlyHMSM(0, 0, 0, LED_SLEEP_TIME, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

void Leds_TaskCreate(void){
    OS_ERR err;

    OSTaskCreate(&LedsTaskTCB,
                 "Leds Task",
                 Leds_Task,
                 0,
                 LEDS_TASK_PRIO,
                 &LedsTaskStk[0],
                 LEDS_TASK_STACKSIZE / 10,
                 LEDS_TASK_STACKSIZE,
                 0,
                 0,
                 0,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);
}

void Led_Control(uint8_t led, bool state) {
    switch(led){
        case 0:     //Sets all LEDs to the same state
            leds[0] = state;
            leds[1] = state;
            leds[2] = state;
            break;
        case 1: //d1
            leds[2] = state;
            break;
        case 2: //d2
            leds[1] = state;
            break;
        case 3: //d3
            leds[0] = state;
            break;
    }
}

void ColorLed_Control(uint8_t led, bool state){
    switch(led){
        case 'A':
            gpioWrite(PIN_LED_RED, state);
            gpioWrite(PIN_LED_GREEN, state);
            gpioWrite(PIN_LED_BLUE, state);
            break;
        case 'R':
            gpioWrite(PIN_LED_RED, state);
            break;
        case 'G':
            gpioWrite(PIN_LED_GREEN, state);
            break;
        case 'B':
            gpioWrite(PIN_LED_BLUE, state);
            break;
    }
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
*******************************************************************************
******************************************************************************/

void Led_Set(uint8_t led) {
    switch (led) {
        case 0: //d3
            gpioWrite(STATUS0, HIGH);
            gpioWrite(STATUS1, HIGH);
            break;
        case 1: //d2
            gpioWrite(STATUS0, LOW);
            gpioWrite(STATUS1, HIGH);
            break;
        case 2: //d1
            gpioWrite(STATUS0, HIGH);
            gpioWrite(STATUS1, LOW);
            break;
        case 3:  // All LEDs OFF
            gpioWrite(STATUS0, LOW);
            gpioWrite(STATUS1, LOW);
            break;
    }
}