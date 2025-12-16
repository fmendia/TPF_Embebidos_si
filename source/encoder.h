/***************************************************************************//**
@file     encoder.h
@brief    Rotary encoder management
@author   Emma Herrera
******************************************************************************/

#ifndef _ENCODER_H_
#define _ENCODER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <os.h>
#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * RTOS VARIABLES
 ******************************************************************************/
#define ENCODER_TASK_PRIO      5u
#define ENCODER_TASK_STACKSIZE 256u

extern OS_TCB   EncoderTaskTCB;
extern CPU_STK  EncoderTaskStk[ENCODER_TASK_STACKSIZE];

// Bits for different events			//a agregar con card
#define ENCODER_FLAG_MOVE   0x01
#define ENCODER_FLAG_BUTTON 0x02

extern OS_FLAG_GRP InputEvents;   // shared event flag group
/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes the rotary encoder pins
 * @note  Sets the pins as INPUT for the rotary encoder and INPUT for the push button
 */
void Encoder_Init (void);

/**
 * @brief Creates encoder task
 */
void Encoder_TaskCreate(void);
void EncoderTask(void *p_arg);
/**
 * @brief Reads rotary encoder turns
 * @return number of turns and direction (negative left turn, positive right turn)
 */
int32_t encoder_turns();

/**
 * @brief Reads the state of the push button
 * @return if the button was pressed
 */
bool Encoder_ButtonWasPressed(void);

/**
 * @brief Reads if the push button is being pressed
 * @return present state of the button (pressed or not pressed)
 */
bool Encoder_ButtonIsPressed(void);

/**
 * @brief Reads the duration of the push button press
 * @return duration of the button press in cicles
 */
uint32_t Encoder_ButtonGetPressDuration(void);

/*******************************************************************************
 ******************************************************************************/

#endif // _ENCODER_H_
