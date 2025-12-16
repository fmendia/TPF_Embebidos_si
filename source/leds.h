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

#include <stdint.h>
#include <stdbool.h>

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

//PUBLICO SÓLO PARA TESTEO RTOS ENCODER - BORRAR
void Led_Set(uint8_t led);

/*******************************************************************************
 ******************************************************************************/

#endif // _LEDS_H_
