/*
 * ui.c
 *
 * Crea cola + pool + tarea UI.
 * UITask hace OSQPend con timeout 200ms para permitir Menu_Tick blink.
 *
 * Integración:
 *   - Menu_Init() es llamada desde UI_Init()
 *   - Al recibir mensaje: Menu_HandleMessage(pmsg); MsgPool_Free(pmsg)
 *   - Al timeout: Menu_Tick()
 */

#include <os.h>
#include <string.h>
#include "ui.h"
#include "msg_pool.h"
#include "menu.h"

/* Queue size - ajustar si esperás ráfagas */
#define UI_QUEUE_SIZE       12u

static OS_Q ui_q;

/* Exponer cola a productores */
OS_Q *UI_GetQueue(void)
{
    return &ui_q;
}

/* UI Task definitions */
#define UI_TASK_PRIO        3u     /* recomendado: UI alto */
#define UI_TASK_STK_SIZE    512u

static OS_TCB  ui_tcb;
static CPU_STK ui_stk[UI_TASK_STK_SIZE];

static void UITask(void *p_arg)
{
    OS_ERR err;
    UI_Message_t *pmsg;
    OS_MSG_SIZE msg_size;
    CPU_TS ts;

    (void)p_arg;

    for (;;) {
        /* Esperar mensaje con timeout 200 ms */
        pmsg = (UI_Message_t *)OSQPend(&ui_q,
                                       200u, /* ms */
                                       OS_OPT_PEND_BLOCKING,
                                       &msg_size,
                                       &ts,
                                       &err);
        if (err == OS_ERR_NONE && pmsg != NULL) {
            /* Procesar mensaje en la máquina de menú */
            Menu_HandleMessage(pmsg);

            /* Liberar el bloque */
            MsgPool_Free(pmsg, &err);
            (void)err;
        } else if (err == OS_ERR_TIMEOUT) {
            /* Timeout: tick del menú (blink, timers) */
            Menu_Tick();
        } else {
            /* Otros errores: ignorar y continuar */
        }
    }
}

void UI_Init(void)
{
    OS_ERR err;

    /* Crear cola de punteros (msg_size = 0 al postear) */
    OSQCreate(&ui_q,
              "UI Queue",
              UI_QUEUE_SIZE,
              &err);

    /* Crear pool de mensajes */
    MsgPool_Init();

    /* Inicializar subsistema de menú (buffers, estado, render inicial) */
    Menu_Init();

    /* Crear tarea UI */
    OSTaskCreate(&ui_tcb,
                 "UI Task",
                 UITask,
                 0,
                 UI_TASK_PRIO,
                 &ui_stk[0],
                 UI_TASK_STK_SIZE/10u,
                 UI_TASK_STK_SIZE,
                 0u,0u,0u,
                 (OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK),
                 &err);
    (void)err;
}
