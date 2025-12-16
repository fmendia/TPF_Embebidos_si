#ifndef WS2812_H_
#define WS2812_H_

#include <stdint.h>
// Estructura para manejo humano de colores
typedef struct {
    uint8_t G;
    uint8_t R;
    uint8_t B;
} Pixel_t;

void WS2812_Init(void);
void WS2812_Refresh(void);
void Set_LED(int index, uint8_t r, uint8_t g, uint8_t b);
void WS2812_Send(void);


#endif /* WS2812_H_ */
