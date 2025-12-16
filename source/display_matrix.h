#ifndef DISPLAY_MATRIX_H
#define DISPLAY_MATRIX_H


#include <stdint.h>


// Dimensiones matriz
#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8


// Pisos: 16 LEDs (4x4) por piso
#define FLOOR_LED_COUNT 16
#define MAX_FLOORS 4
#define MAX_PEOPLE_PER_FLOOR 4 // Cada persona = bloque 2x2


// Colores predefinidos
typedef enum {
COLOR_OFF = 0,
COLOR_RED,
COLOR_GREEN,
COLOR_BLUE,
COLOR_YELLOW,
COLOR_CYAN,
COLOR_MAGENTA,
COLOR_WHITE
} Color_t;


void Matrix_Init(void);
void Matrix_Clear(void);
void Matrix_AddPerson(uint8_t floor);
void Matrix_RemovePerson(uint8_t floor);

#endif //DISPLAY_MATRIX_H
