#include "FTM.h"
#include "MK64F12.h"

// ----------------------------------------------------------------
void FTM_Init(void) {
    PORTD->PCR[1] = PORT_PCR_MUX(4);

    // FTMEN = 0 (Clave del éxito)
    FTM3->MODE = FTM_MODE_WPDIS_MASK;
    FTM3->MODE &= ~FTM_MODE_FTMEN_MASK;

    FTM3->SC = 0x00;
    FTM3->CNT = 0;
    FTM3->MOD = FTM_MOD_COUNT;

    // Configuración PWM Edge-Aligned + DMA
    FTM3->CONTROLS[FTM_CHANNEL].CnSC = FTM_CnSC_MSB(1) | FTM_CnSC_ELSB(1) |
                                      FTM_CnSC_CHIE(1) | FTM_CnSC_DMA(1);
    // Valor inicial (Reset/Silencio)
    FTM3->CONTROLS[FTM_CHANNEL].CnV = 1;
}
