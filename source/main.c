#include "hardware.h"
#include <os.h>
#include "encoder.h"
#include "leds.h"
#include "card.h"
#include "display_rtos.h"
#include "ui.h"            // <-- NUEVO
#include "msg_pool.h"      // <-- NUEVO
#include "display_matrix.h"

#include <stdint.h>
#include <stdbool.h>

/* LEDs (idéntico a tu archivo original) */
#define LED_R_PORT            PORTB
#define LED_R_GPIO            GPIOB
#define LED_G_PORT            PORTE
#define LED_G_GPIO            GPIOE
#define LED_B_PORT            PORTB
#define LED_B_GPIO            GPIOB
#define LED_R_PIN             22
#define LED_G_PIN             26
#define LED_B_PIN             21

#define LED_B_ON()           (LED_B_GPIO->PCOR |= (1 << LED_B_PIN))
#define LED_B_OFF()          (LED_B_GPIO->PSOR |= (1 << LED_B_PIN))
#define LED_B_TOGGLE()       (LED_B_GPIO->PTOR |= (1 << LED_B_PIN))
#define LED_G_ON()           (LED_G_GPIO->PCOR |= (1 << LED_G_PIN))
#define LED_G_OFF()          (LED_G_GPIO->PSOR |= (1 << LED_G_PIN))
#define LED_G_TOGGLE()       (LED_G_GPIO->PTOR |= (1 << LED_G_PIN))
#define LED_R_ON()           (LED_R_GPIO->PCOR |= (1 << LED_R_PIN))
#define LED_R_OFF()          (LED_R_GPIO->PSOR |= (1 << LED_R_PIN))
#define LED_R_TOGGLE()       (LED_R_GPIO->PTOR |= (1 << LED_R_PIN))

/* TASK START */
#define TASKSTART_STK_SIZE  512u
#define TASKSTART_PRIO      10u

static OS_TCB  TaskStartTCB;
static CPU_STK TaskStartStk[TASKSTART_STK_SIZE];

/* -------------------------------------------------------------------------- */

static void TaskStart(void *p_arg)
{
    (void)p_arg;
    OS_ERR err;

    CPU_Init();

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif


    //Init Hardware
    Encoder_Init();
    Leds_Init();
    card_init();
    Display_Init();
    display_brightness(3);
    Matrix_Init();
    // Init UI y Message Queue
    UI_Init();

    //Tasks init
    Encoder_TaskCreate();
    Card_TaskCreate();
    Display_TaskCreate();

    //Heartbeat, para ver que sigue corriendo
    while (1) {
       // static bool led_state = false;
       // led_state = !led_state;
       // Led_Control(0, led_state);

       // OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);
    }
}

/* -------------------------------------------------------------------------- */

int main(void)
{
    OS_ERR err;

    /* Inicializaciones de bajo nivel */
    hw_Init();

    /* ------------------------------------------------------------------ */

    OSInit(&err);

#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
    OSSchedRoundRobinCfg((CPU_BOOLEAN)1, 0, &err);
#endif

    OS_CPU_SysTickInit(SystemCoreClock / (uint32_t)OSCfg_TickRate_Hz);

    /* Crear TaskStart */
    OSTaskCreate(&TaskStartTCB,
                 "App Task Start",
                 TaskStart,
                 0u,
                 TASKSTART_PRIO,
                 &TaskStartStk[0u],
                 TASKSTART_STK_SIZE / 10u,
                 TASKSTART_STK_SIZE,
                 0u,
                 0u,
                 0u,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 &err);

    /* Lanzar scheduler */
    OSStart(&err);

    /* Nunca vuelve */
    while (1) { }
}
