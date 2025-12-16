/***************************************************************************//**
  @file     card.h
  @brief    Card management
  @author   Santiago Cámara
 ******************************************************************************/

#ifndef _CARD_H_
#define _CARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <os.h>
#include <stdio.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define MAX_ID_SIZE 9                          //Tamaño máximo del array de chars (se cuenta el terminador)

/*******************************************************************************
 * RTOS VARIABLES
 ******************************************************************************/
#define CARD_TASK_PRIO      6u
#define CARD_TASK_STACKSIZE 512u
#define CARD_VALID_DATA    0x03u

extern OS_TCB   CardTaskTCB;
extern CPU_STK  CardTaskStk[CARD_TASK_STACKSIZE];
extern OS_FLAG_GRP InputEvents;   // shared event flag group
//SANTI: Verificar aca el uso de flags
// Bits for different events			//a agregar con card
// #define ENCODER_FLAG_MOVE   0x01
// #define ENCODER_FLAG_BUTTON 0x02

//extern OS_FLAG_GRP InputEvents;   // shared event flag group
/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes the card reader
 * @note  Needs a PIN for ENABLE, CLOCK and DATA. All PINs are configured as INPUT.
 */
void card_init (void);

void Card_TaskCreate(void);

void CardTask(void *p_arg);


/**
 * @brief Checks if there is valid data from the card reader
 *
 * @return true if there is valid data, false otherwise
 */
bool check_card_new_data(void);

/**
 * @brief Returns the ID of the card
 * @note  Its a pointer to an array of chars, so it can be used as a string, that has size MAX_ID_SIZE.
 */
char* get_card_id(void);

#endif // _CARD_H_
