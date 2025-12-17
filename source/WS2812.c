#include "WS2812.h"
#include "MK64F12.h"
#include "FTM.h"
#include "hardware.h"
#include "os.h"

#define LED_COUNT       64      // 8x8 LEDs
#define RESET_SLOTS     40      // 40 slots de 1.25us = 50us de silencio (Reset > 50us)
#define TOTAL_BUFF_SIZE ((LED_COUNT * 24))

// Buffer DMA (Salida cruda al Timer)
uint32_t pwm_buffer[TOTAL_BUFF_SIZE];
Pixel_t ledsM[LED_COUNT];
uint8_t ws_busy = 0;
void DMA_Init();
void hw_Init2();
void WS2812_Init(void) {
	//hw_Init();
	hw_Init2();
    // Inicializar todo apagado
    for(int i=0; i<LED_COUNT; i++) Set_LED(i, 0, 0, 0);
    WS2812_Refresh();

    DMA_Init();
    FTM_Init();
    
    WS2812_Send();
    
}

static inline void DMA_Rearm(void)
{
    DMA0->TCD[0].SADDR = (uint32_t)&pwm_buffer[0];
    DMA0->TCD[0].CITER_ELINKNO = TOTAL_BUFF_SIZE;
    DMA0->TCD[0].BITER_ELINKNO = TOTAL_BUFF_SIZE;
}


void WS2812_Refresh(void) {
    uint32_t buff_idx = 0;

    for (int i = 0; i < LED_COUNT; i++) {
        // Orden WS2812B: GRB (Green, Red, Blue) - MSB First

        // 1. GREEN
        for (int bit = 7; bit >= 0; bit--) {
            if ((ledsM[i].G >> bit) & 0x01) pwm_buffer[buff_idx++] = FTM_VAL_1;
            else                           pwm_buffer[buff_idx++] = FTM_VAL_0;
        }
        // 2. RED
        for (int bit = 7; bit >= 0; bit--) {
            if ((ledsM[i].R >> bit) & 0x01) pwm_buffer[buff_idx++] = FTM_VAL_1;
            else                           pwm_buffer[buff_idx++] = FTM_VAL_0;
        }
        // 3. BLUE
        for (int bit = 7; bit >= 0; bit--) {
            if ((ledsM[i].B >> bit) & 0x01) pwm_buffer[buff_idx++] = FTM_VAL_1;
            else                           pwm_buffer[buff_idx++] = FTM_VAL_0;
        }
    }
}

// Función para setear un pixel en memoria (no actualiza LEDs aun)
void Set_LED(int index, uint8_t r, uint8_t g, uint8_t b) {
    if(index < 0 || index >= LED_COUNT) return;
    ledsM[index].R = r;
    ledsM[index].G = g;
    ledsM[index].B = b;
}

// Retardo simple
void Delay(uint32_t cycles) {
    while(cycles--) __asm("nop");
}

void WS2812_Send(void)
{
    //CPU_SR_ALLOC();
    //CPU_CRITICAL_ENTER();

    //DMA_Rearm();

    //DMA0->INT = (1 << 0);      // Limpio flag
    //DMA0->ERQ |= (1 << 0);     // Habilito request
    //FTM3->CONTROLS[FTM_CHANNEL].CnV = 1;
    FTM3->CNT = 0;
    FTM3->SC = FTM_SC_CLKS(1);

    //CPU_CRITICAL_EXIT();
}


void DMA_Init(void) {
    DMAMUX->CHCFG[0] = 0;

    // CHANGE: Source 23 is FTM0. Source 37 is FTM3 Channel 1.
    DMAMUX->CHCFG[0] = DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(33);

    DMA0->TCD[0].SADDR = (uint32_t)(&pwm_buffer[0]);
    DMA0->TCD[0].SOFF = 4; // 4 bytes offset (uint32_t)

    DMA0->TCD[0].ATTR = DMA_ATTR_SSIZE(2) | DMA_ATTR_DSIZE(2);
    DMA0->TCD[0].NBYTES_MLNO = 4;

    // This line is already correct in your code, but verify FTM_CHANNEL is 1
    DMA0->TCD[0].DADDR = (uint32_t)&FTM3->CONTROLS[FTM_CHANNEL].CnV;
    DMA0->TCD[0].DOFF = 0;

    DMA0->TCD[0].CITER_ELINKNO = TOTAL_BUFF_SIZE;
    DMA0->TCD[0].BITER_ELINKNO = TOTAL_BUFF_SIZE;

    DMA0->TCD[0].SLAST = -(sizeof(pwm_buffer));
    DMA0->TCD[0].DLAST_SGA = 0;
    DMA0->TCD[0].CSR = DMA_CSR_INTMAJOR_MASK;
    DMA0->ERQ |= (1 << 0);
    NVIC_EnableIRQ(DMA0_IRQn);
}

void DMA0_IRQHandler(void)
{
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();
    DMA0->CINT |= (1<<6);
    //DMA0->CINT |= 0; // LIMPIAR FLAG PRIMERO
    FTM3->SC = 0;           // stop timer
    //DMA0->ERQ &= ~(1 << 0);
   // FTM3->CONTROLS[FTM_CHANNEL].CnV = 0; // FORZAR LOW

    OSIntExit();
}

void hw_Init2 (void) {
    // Tu configuración de reloj original (Correcta)
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));
    WDOG->UNLOCK  = WDOG_UNLOCK_WDOGUNLOCK(0xC520);
    WDOG->UNLOCK  = WDOG_UNLOCK_WDOGUNLOCK(0xD928);
    WDOG->STCTRLH = WDOG_STCTRLH_ALLOWUPDATE_MASK | WDOG_STCTRLH_CLKSRC_MASK | 0x0100U;
    PMC->REGSC |= PMC_REGSC_ACKISO_MASK;
    SIM->CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0x00) | SIM_CLKDIV1_OUTDIV2(0x01) | SIM_CLKDIV1_OUTDIV3(0x01) | SIM_CLKDIV1_OUTDIV4(0x03);
    SIM->SOPT1 |= SIM_SOPT1_OSC32KSEL(0x03);
    SIM->SOPT2 = SIM_SOPT2_PLLFLLSEL_MASK;
    MCG->SC = MCG_SC_FCRDIV(0x02);
    MCG->C2 = MCG_C2_RANGE(0x02);
    OSC->CR = OSC_CR_ERCLKEN_MASK;
    MCG->C7 = MCG_C7_OSCSEL(0x00);
    MCG->C1 = MCG_C1_CLKS(0x02) | MCG_C1_FRDIV(0x07);
    while((MCG->S & MCG_S_IREFST_MASK) != 0x00U);
    MCG->C5 = MCG_C5_PRDIV0(0x0F);
    MCG->C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV0(0x08);
    while((MCG->S & MCG_S_LOCK0_MASK) == 0x00U);
    MCG->C1 &= ~MCG_C1_CLKS_MASK;
    while((MCG->S & MCG_S_CLKST_MASK) != 0x0CU);

    SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK | SIM_SCGC6_DMAMUX_MASK;
    SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
    SIM->SCGC6 |= SIM_SCGC6_FTM1_MASK;
	SIM->SCGC6 |= SIM_SCGC6_FTM2_MASK;
	SIM->SCGC3 |= SIM_SCGC3_FTM2_MASK;
	SIM->SCGC3 |= SIM_SCGC3_FTM3_MASK;
}
