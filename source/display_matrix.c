#include "display_matrix.h"
#include "WS2812.h"

// ---------------- CONFIG ----------------

static const Color_t floor_color[MAX_FLOORS] = {
    COLOR_RED,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_YELLOW
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

// Dibuja una persona (2x2) dentro de un piso (4x4)
static void Draw_Person(uint8_t floor, uint8_t person, Color_t color)
{
    // Piso -> fila base
    uint8_t floor_row = (floor / 2) * 4;
    uint8_t floor_col = (floor % 2) * 4;

    // Persona dentro del piso
    uint8_t px = (person % 2) * 2;
    uint8_t py = (person / 2) * 2;

    uint8_t base_row = floor_row + py;
    uint8_t base_col = floor_col + px;

    for (uint8_t y = 0; y < 2; y++) {
        for (uint8_t x = 0; x < 2; x++) {
            uint8_t idx = (base_row + y) * 8 + (base_col + x);
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
        Set_LED(i, r, g, b);
    }

    WS2812_Refresh();
    WS2812_Send();
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
   // if (floor >= MAX_FLOORS) return;
   // if (people_per_floor[floor] >= MAX_PEOPLE_PER_FLOOR) return;
    uint8_t r,g,b;
    switch(floor){
    case 0:
    	r =55;g=0;b=0;
    	break;
    case 1:
    	r =0;g=55;b=0;
    	break;
    case 2:
		r =0;g=0;b=55;
		break;
	case 3:
		r =55;g=0;b=55;
		break;
	default: r=0; g=0;b=0;
	break;
    }

		Set_LED(floor, r,g,b);
		WS2812_Refresh();
		WS2812_Send();

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
