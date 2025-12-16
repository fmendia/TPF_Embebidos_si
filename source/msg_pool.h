#ifndef MSG_POOL_H
#define MSG_POOL_H

#include <os.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define UI_MSG_POOL_SIZE  16u    /* número de bloques en el pool */
#define UI_MSG_ID_SIZE    16u    /* suficiente para IDs/PINs, aunque ID usado es 8 */

typedef enum {
    UI_EVENT_ENCODER_MOVE = 0,
    UI_EVENT_ENCODER_BUTTON,
    UI_EVENT_ENCODER_LONGPRESS,  /* nuevo: long press -> backspace */
    UI_EVENT_CARD_READ,
} UI_EventType_t;

/* El bloque de mensaje que usamos: tamaño fijo */
typedef struct {
    UI_EventType_t type;
    uint32_t       timestamp_ms; /* opcional */
    union {
        int32_t    encoder_delta;    // for ENCODER_MOVE
        bool       button_pressed;   // for BUTTON
        char       card_id[UI_MSG_ID_SIZE];  // for CARD_READ (>=8)
    } data;
} UI_Message_t;

/* Inicializa pool (llamar desde UI_Init) */
void MsgPool_Init(void);

/* obtener / liberar bloque (OSMemGet / OSMemPut wrappers) */
UI_Message_t * MsgPool_Alloc(OS_ERR *p_err); /* ISR-safe (no block) */
void           MsgPool_Free(UI_Message_t *p_msg, OS_ERR *p_err);

#endif /* MSG_POOL_H */
