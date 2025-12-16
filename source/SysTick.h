/***************************************************************************//**
  @file     SysTick.h
  @brief    SysTick driver
  @author   Nicol�s Magliola
 ******************************************************************************/

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SYSTICK_ISR_FREQUENCY_HZ 2000U //Value between 6HZ and 100MHz


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

 /**
  * @brief Manages the SysTick timer and registers a callback function to be called after a specified number of ticks.
  *
  * @param funcallback Pointer to the function to be called when the SysTick interrupt occurs
  * @param ticks Number of ticks to wait before calling the function
  * @return true if the function was successfully registered, false otherwise
  */
 bool SysTick_Manager(void (*funcallback)(void), uint32_t ticks);


  /**
    * @brief Gets the frequency of the SysTick timer and registers a callback function.
    *
    * @param funcallback Pointer to the function whose frequency is to be measured
    * @return Frequency wich the function is called in Hz
    */
 float SysTick_GetFrec(void (*funcallback)(void));
/*******************************************************************************
 ******************************************************************************/

#endif // _SYSTICK_H_
