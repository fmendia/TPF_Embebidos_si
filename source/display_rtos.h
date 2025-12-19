/***************************************************************************//**
@file     display.h
@brief    Display management
@author   Emma Herrera
******************************************************************************/

#ifndef _DISPLAY_RTOS_H_
#define _DISPLAY_RTOS_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <os.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define NUMBER_DISPLAYS 4 // Number of displays

enum brightness_level {
    VERY_LOW = 1,
    LOW,
    MEDIUM,
    HIGH,
    VERY_HIGH
};


enum led_list{
	RED = 0,
	GREEN,
	BLUE,
	NONE
};

void led_select(uint8_t led);
static uint8_t selected_led =0;
//Task display
#define TASKDISP_STK_SIZE 256u
#define TASKDISP_STK_SIZE_LIMIT	(TASKDISP_STK_SIZE / 10u)
#define TASKDISP_PRIO 8u
static OS_TCB TaskDisplayTCB;
static CPU_STK TaskDisplayStk[TASKDISP_STK_SIZE];

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
static void TaskDisplay(void *p_arg);
void Display_TaskCreate(void);
void num2disp(uint32_t num);
void display_select(uint8_t digit);
/**
 * @brief Initializes the display pins
 * @note  Sets the pins as OUTPUT for the display and selector pins
 */
void Display_Init (void);

/**
 * @brief Displays a string on the 7-segment display
 * @param str Pointer to the string to be displayed
 * @param rst Boolean value to reset the display
 * @note  The function scrolls the string across the display
 */
void str2disp(uint8_t* str, bool rst);

/**
 * @brief Displays a MINIMUM 4 CHARACTER LONG string on the 7-segment display
 * @param str Pointer to the string to be displayed
 * @param rst Boolean value to reset the display
 * @note  The function shows last 4 digits of input and blinks the last one
 * @note The function does not verify that the string has more than 4 characters
 */
void input2disp(uint8_t* str);


/**
 * @brief Displays a single character on the 7-segment display on a specific position
 * @param digit Character to be displayed
 * @param position Position on the display (0-3)
 */
void display_char(uint8_t digit, uint8_t position);
void display_string4(const char* str);
/**
 * @brief Sets the brightness of the display
 * @param brightness Brightness level (0-255)
 */
void display_brightness(uint8_t brightness);

/**
 * @brief Clears the display
 */
void clear_display(void);

/**
 * @brief Clears a specific digit on the display
 * @param position Position of the digit to be cleared (0-3)
 */
void clear_digit(uint8_t position);

/**
 * @brief Shifts the display content in a specific direction
 * @param direction Direction to shift (0 for left, 1 for right)
 * @param shift_amount Number of positions to shift
 */
void shift_display(uint8_t direction, uint8_t shift_amount);

/**
 * @brief Repositions the display content to a new digit
 * @param direction Direction to shift (0 for left, 1 for right)
 * @param new_digit New digit to be displayed
 */
void shift_display_reposition(uint8_t direction, char new_digit);

/**
 * @brief Refreshes the display
 * @note  This function is called periodically to update the display content
 */
void refresh_display(void);

/**
 * @brief Scrollea mensaje
 * @note  Cada vez que se la llama hace shift a izq el display
 * @param mensaje
 */
void ScrollMessage(char *message);

void ResetScroll(void);

/*******************************************************************************
 ******************************************************************************/

#endif // _DISPLAY_RTOS_H_
