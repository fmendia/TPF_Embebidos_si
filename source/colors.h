#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Color_t;

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
    COLOR_PURPLE,
    COLOR_PURPLE_LIGHT,
    COLOR_TEAL,
    COLOR_LIME,
    COLOR_INDIGO,
    COLOR_CORAL,
    COLOR_TURQUOISE,
    COLOR_GOLD,
    COLOR_NAVY,
    COLOR_MINT,
    COLOR_ENUM_COUNT
} ColorEnum_t;

// Tabla de colores NORMALIZADA (Full Range 0-255)
// Se mantiene la proporción (Hue) original, pero maximizando el brillo base.
static const Color_t COLOR_MAP[COLOR_ENUM_COUNT] = {
    {0, 0, 0},              // COLOR_OFF

    // COLORES BASICOS (Saturación Máxima)
    {255, 0, 0},            // COLOR_RED
    {0, 255, 0},            // COLOR_GREEN
    {0, 0, 255},            // COLOR_BLUE
    {255, 220, 0},          // COLOR_YELLOW
    {255, 100, 0},          // COLOR_ORANGE
    {0, 255, 255},          // COLOR_CYAN
    {255, 0, 255},          // COLOR_MAGENTA
    {255, 255, 255},        // COLOR_WHITE

    // VARIACIONES DE NARANJA
    {255, 140, 40},         // COLOR_ORANGE_LIGHT (Pastel)
    {255, 80, 0},           // COLOR_ORANGE_MEDIUM
    {255, 40, 0},           // COLOR_ORANGE_DARK (Más rojizo)
    {255, 10, 0},           // COLOR_ORANGE_DARKER (Casi rojo)

    // VARIACIONES DE ROSA
    {255, 100, 180},        // COLOR_PINK_LIGHT
    {255, 50, 100},         // COLOR_PINK_MEDIUM
    {255, 0, 80},           // COLOR_PINK_DARK
    {255, 0, 40},           // COLOR_PINK_DARKER

    // VARIACIONES DE AMARILLO/VERDE
    {200, 255, 0},          // COLOR_YELLOW_LIGHT (Lima limón)
    {150, 255, 0},          // COLOR_YELLOW_MEDIUM
    {80, 255, 0},           // COLOR_YELLOW_DARK
    {40, 255, 0},           // COLOR_YELLOW_DARKER

    // COLORES ADICIONALES
    {150, 0, 255},          // COLOR_PURPLE
    {200, 0, 255},          // COLOR_PURPLE_LIGHT
    {0, 128, 128},          // COLOR_TEAL (Este suele ser medio brillo por definición)
    {191, 255, 0},          // COLOR_LIME
    {75, 0, 130},           // COLOR_INDIGO
    {255, 127, 80},         // COLOR_CORAL
    {64, 224, 208},         // COLOR_TURQUOISE
    {255, 215, 0},          // COLOR_GOLD
    {0, 0, 128},            // COLOR_NAVY (Azul oscuro, ratio mantenido)
    {170, 255, 195}         // COLOR_MINT
};

#endif //COLORS_H