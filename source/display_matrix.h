#ifndef DISPLAY_MATRIX_H
#define DISPLAY_MATRIX_H


#include <stdint.h>
#include "colors.h"

// Dimensiones matriz
#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8


// Pisos: 16 LEDs (4x4) por piso
#define FLOOR_LED_COUNT 16
#define MAX_FLOORS 3
#define MAX_PEOPLE_PER_FLOOR 4 // Cada persona = bloque 2x2

#define SIGNAL_MENU_USER 0
#define SIGNAL_MENU_ADMIN 1
#define SIGNAL_OLD_PIN 2
#define SIGNAL_NEW_PIN 3
#define SIGNAL_OFF 4

/**
 * @brief Set the color of a specific LED in the matrix
 * 
 * @param x The x coordinate (0 to MATRIX_WIDTH-1)
 * @param y The y coordinate (0 to MATRIX_HEIGHT-1)
 * @param color The color to set
 */
void Matrix_PaintLed(uint8_t x, uint8_t y, Color_t color);

/**
 * @brief Initialize the matrix display
 * 
 */
void Matrix_Init(void);

/**
 * @brief Clear the entire matrix display
 * 
 */
void Matrix_Clear(void);

/**
 * @brief Add a person to the specified floor
 * 
 * @param floor Floor number (0 to MAX_FLOORS-1)
 */
void Matrix_AddPerson(uint8_t floor);

/**
 * @brief Remove a person from the specified floor
 * 
 * @param floor Floor number (0 to MAX_FLOORS-1)
 */
void Matrix_RemovePerson(uint8_t floor);

/**
* @brief Cambio brillo de matriz
*
* @param bri mivel de brillo
* */
void Matrix_Brightness(uint8_t bri);

void Matrix_Restore(void);

/**
 * @brief Indicate a signal on the matrix (e.g., menu selection, PIN status)
 * 
 * @param signal The signal to display
 */
void Matrix_Signal_Floor(uint8_t signal);

#endif //DISPLAY_MATRIX_H
