#ifndef COLORS_H
#define COLORS_H
// Estructura de color RGB
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Color_t;

// Enum de colores predefinidos con tabla RGB asociada
typedef enum {
    COLOR_OFF = 0,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_ORANGE,
    COLOR_CYAN,
    COLOR_MAGENTA,
    COLOR_WHITE,
    COLOR_ORANGE_LIGHT,
    COLOR_ORANGE_MEDIUM,
    COLOR_ORANGE_DARK,
    COLOR_ORANGE_DARKER,
    COLOR_PINK_LIGHT,
    COLOR_PINK_MEDIUM,
    COLOR_PINK_DARK,
    COLOR_PINK_DARKER,
    COLOR_YELLOW_LIGHT,
    COLOR_YELLOW_MEDIUM,
    COLOR_YELLOW_DARK,
    COLOR_YELLOW_DARKER,
    COLOR_ENUM_COUNT
} ColorEnum_t;

// Tabla de colores RGB indexada por ColorEnum_t
static const Color_t COLOR_MAP[COLOR_ENUM_COUNT] = {
    {0, 0, 0},            // COLOR_OFF

    // COLORES PUROS (Brillo base ~120-140)
    {130, 0, 0},          // COLOR_RED
    {0, 120, 0},          // COLOR_GREEN (El verde brilla más, por eso se baja el valor)
    {0, 0, 150},          // COLOR_BLUE (El azul brilla menos, se sube el valor)
    {70, 60, 0},          // COLOR_YELLOW
    {100, 30, 0},         // COLOR_ORANGE
    {0, 70, 70},          // COLOR_CYAN
    {70, 0, 70},          // COLOR_MAGENTA
    {45, 45, 45},         // COLOR_WHITE

    // VARIACIONES DE NARANJA (Todos suman ~100-110 de brillo percibido)
    {75, 30, 5},          // COLOR_ORANGE_LIGHT (Más pastel, pero mismo brillo)
    {90, 20, 0},          // COLOR_ORANGE_MEDIUM
    {105, 5, 0},          // COLOR_ORANGE_DARK (Más saturado de rojo, mismo brillo)
    {110, 0, 0},          // COLOR_ORANGE_DARKER (Casi rojo fuego)

    // VARIACIONES DE ROSA (Todos mantienen la misma intensidad de luz)
    {65, 10, 35},         // COLOR_PINK_LIGHT
    {80, 10, 20},         // COLOR_PINK_MEDIUM
    {100, 0, 20},         // COLOR_PINK_DARK
    {110, 0, 10},         // COLOR_PINK_DARKER

    // VARIACIONES DE AMARILLO/VERDE
    {50, 60, 0},          // COLOR_YELLOW_LIGHT (Más hacia el verde/limón)
    {40, 70, 0},          // COLOR_YELLOW_MEDIUM
    {20, 90, 0},          // COLOR_YELLOW_DARK
    {10, 100, 0},         // COLOR_YELLOW_DARKER
};

#endif //COLORS_H

