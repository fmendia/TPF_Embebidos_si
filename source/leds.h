/***************************************************************************//**
@file     leds.h
@brief    Leds management
@author   Grupo 1
******************************************************************************/

#ifndef _LEDS_H_
#define _LEDS_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <os.h>
#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * RTOS VARIABLES
 ******************************************************************************/
#define LEDS_TASK_PRIO      4u		//DEFINIR BIEN!
#define LEDS_TASK_STACKSIZE 256u

extern OS_TCB   LedsTaskTCB;
extern CPU_STK  LedsTaskStk[LEDS_TASK_STACKSIZE];

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes the LED pins
 * @note  Sets the pins as OUTPUT for the LED pins
 */
void Leds_Init(void);

/**
 * @brief Controls the state of a specific LED
 * @param led The LED to control (0 for all, 3 for d3, 2 for d2, 1 for d1)
 * @param state The desired state (true for ON, false for OFF)
 */
void Led_Control(uint8_t led, bool state);

/**
 * @brief Controls the state of a specific LED color
 * @param led The LED to control ('A' for all, 'R' for red, 'G' for green, 'B' for blue)
 * @param state The desired state (true for ON, false for OFF)
 */
void ColorLed_Control(uint8_t led, bool state);

/**
 * @brief Creates LED task
 */
void Leds_TaskCreate(void);

/**
 * @brief LED task function
 */
void Leds_Task(void *p_arg);

/*******************************************************************************
 ******************************************************************************/

#endif // _LEDS_H_