/*
*File description: SysTick.c is the source file for the SysTick driver.
*This file contains the functions for the SysTick driver.
*/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "SysTick.h"
#include "MK64F12.h"
#include "hardware.h"
#include "gpio.h"
#include <stddef.h>

#define SYSTICK_CLOCK_HZ 100000000  // 100 MHz SysTick clock

#define MAX_CALLBACKS 8

 /*******************************************************************************
  * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
  ******************************************************************************/

typedef struct {
    void (*callback)(void); // Pointer to the callback function
    uint32_t ticks;          // Number of ticks for the callback
    uint32_t count;         // Counter to keep track of ticks
} CallbackInfo;

static CallbackInfo callbacks[MAX_CALLBACKS]; // Array to hold callback information

static int callback_count = 0;
static bool initialized = false; // Flag to check if SysTick is initialized

bool SysTick_Init (void);

 /*******************************************************************************
  *******************************************************************************
                         GLOBAL FUNCTION DEFINITIONS
  *******************************************************************************
  ******************************************************************************/


bool SysTick_Manager(void (*funcallback)(void), uint32_t ticks){
    if(initialized == false){
        SysTick_Init(); //Initialize SysTick if not already initialized
        initialized = true; //Set the flag to true
    }
    if (callback_count < MAX_CALLBACKS && funcallback != NULL) {
        callbacks[callback_count].callback = funcallback; // Store the callback function
        callbacks[callback_count].ticks = ticks;         // Store the number of ticks
        callbacks[callback_count].count = 0;             // Initialize the count to 0
        callback_count++;                                 // Increment the callback count
        return true;                                     // Return success
    } else {
        return false;                                    // Return failure if max callbacks reached
    }
}

float SysTick_GetFrec(void (*funcallback)(void)) {
    for (int i = 0; i < callback_count; i++) {
        if (callbacks[i].callback == funcallback) {
            return (float)SYSTICK_ISR_FREQUENCY_HZ / (float)callbacks[i].ticks; // Calculate the frequency based on the ticks
        }
    }
    return 0.0f; // Return 0 if the callback is not found
}

 /*******************************************************************************
  *******************************************************************************
                         LOCAL FUNCTION DEFINITIONS
  *******************************************************************************
  ******************************************************************************/

bool SysTick_Init (void){

   // hw_DisableInterrupts(); //Disable interrupts

    //SysTick initialization
    SysTick->CTRL = 0x00; //Clear the control and status register
    SysTick->LOAD = (uint32_t)(SYSTICK_CLOCK_HZ/SYSTICK_ISR_FREQUENCY_HZ) - 1; //Set the RELOAD register to generate an interrupt according to the SysTick_ISR_FREQUENCY_HZ
    SysTick->VAL = 0x00; //Clear current value register
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk //Selects the clock source as internal
                    | SysTick_CTRL_TICKINT_Msk //Enables the SysTick exception request
                    | SysTick_CTRL_ENABLE_Msk; //Enables the counter

    gpioMode(PORTNUM2PIN(PB,9), OUTPUT);

   // hw_EnableInterrupts(); //Enable interrupts
    return true;
}

 /*******************************************************************************
  ******************************************************************************/

/*__ISR__ SysTick_Handler(void) {
	gpioWrite(PORTNUM2PIN(PB,9), HIGH);
    for (int i = 0; i < callback_count; i++) {
        callbacks[i].count++; // Increment the count for each callback
        if (callbacks[i].count >= callbacks[i].ticks) { // Check if the ticks have been reached
            callbacks[i].callback(); // Call the callback function
            callbacks[i].count = 0; // Reset the count
        }
    }
    gpioWrite(PORTNUM2PIN(PB,9), LOW);
}*/
