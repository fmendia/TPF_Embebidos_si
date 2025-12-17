#include "display_matrix.h"
#include "WS2812.h"

// ---------------- CONFIG ----------------

static const Color_t floor_color[MAX_FLOORS] = {
    COLOR_RED,
    COLOR_BLUE,
    COLOR_GREEN
};

// Estado
static uint8_t people_per_floor[MAX_FLOORS];
static Color_t matrix_state[64];

// ---------------- COLOR MAP ----------------

static void Color_To_RGB(Color_t c, uint8_t *r, uint8_t *g, uint8_t *b)
{
    switch (c) {
        case COLOR_RED:     *r = 20; *g = 0;  *b = 0;  break;
        case COLOR_GREEN:   *r = 0;  *g = 20; *b = 0;  break;
        case COLOR_BLUE:    *r = 0;  *g = 0;  *b = 20; break;
        case COLOR_YELLOW:  *r = 20; *g = 20; *b = 0;  break;
        case COLOR_CYAN:    *r = 0;  *g = 20; *b = 20; break;
        case COLOR_MAGENTA: *r = 20; *g = 0;  *b = 20; break;
        case COLOR_WHITE:   *r = 20; *g = 20; *b = 20; break;
        default:            *r = 0;  *g = 0;  *b = 0;  break;
    }
}

// ---------------- UTIL ----------------

// floor: 0, 1, o 2
// person: 1 a 4 (1 es la de más abajo)
// color: El color para encender
static void Draw_Person(uint8_t floor, uint8_t person, Color_t color)
{
    // 1. VALIDACIÓN
    // Ahora floor va de 0 a 2.
    if (floor > 2) return; 
    if (person < 1 || person > 4) return;

    // 2. CÁLCULO DE COLUMNAS (EJE X)
    // Piso 0 -> Col 0 (0*2)
    // Piso 1 -> Col 2 (1*2)
    // Piso 2 -> Col 4 (2*2)
    uint8_t base_col = floor * 2;  // <--- CAMBIO AQUÍ: Directo, sin restar nada.

    // 3. CÁLCULO DE FILAS (EJE Y)
    // Restamos 1 a 'person' para que la persona 1 sea el offset 0
    uint8_t start_row = (person - 1) * 2;

    // 4. DIBUJO
    for (uint8_t y = 0; y < 2; y++) {
        for (uint8_t x = 0; x < 2; x++) {
            uint8_t row = start_row + y;
            uint8_t col = base_col + x;
            uint8_t idx = (row * 8) + col;

            matrix_state[idx] = color;
        }
    }
}

// Push completo al WS2812
static void Matrix_Push(void)
{
    uint8_t r, g, b;

    for (uint8_t i = 0; i < 64; i++) {
        Color_To_RGB(matrix_state[i], &r, &g, &b);
        Set_LED(i, r, g, b); // Actualiza el buffer interno
    }

    WS2812_Refresh(); // Prepara el buffer DMA
    WS2812_Send(); // Envía los datos al LED strip
}

// ---------------- API ----------------

void Matrix_Init(void)
{
    WS2812_Init();
    //Matrix_Clear();
}

void Matrix_Clear(void)
{
    for (uint8_t i = 0; i < 64; i++)
        matrix_state[i] = COLOR_OFF;

    for (uint8_t f = 0; f < MAX_FLOORS; f++)
        people_per_floor[f] = 0;

    Matrix_Push();
}

void Matrix_AddPerson(uint8_t floor)
{
   if (floor >= MAX_FLOORS) return; // Validar piso
   if (people_per_floor[floor] >= MAX_PEOPLE_PER_FLOOR) return; // Validar espacio
   
   people_per_floor[floor]++;
    Draw_Person(floor,
                people_per_floor[floor],
                floor_color[floor]);
    Matrix_Push();

}
void Matrix_RemovePerson(uint8_t floor)
{
    if (floor >= MAX_FLOORS) return;
    if (people_per_floor[floor] == 0) return;

    Draw_Person(floor,
                people_per_floor[floor],
                COLOR_OFF);
    people_per_floor[floor]--;

    Matrix_Push();
}
