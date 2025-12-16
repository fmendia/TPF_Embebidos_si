#include "msg_pool.h"
#include <string.h>

/* control block y almacenamiento estático */
static OS_MEM   uiMsgMem;
static UI_Message_t uiMsgStorage[UI_MSG_POOL_SIZE];

void MsgPool_Init(void)
{
    OS_ERR err;

    /* Crear partición de memoria para bloques UI_Message_t */
    OSMemCreate(&uiMsgMem,
                "UI Msg Pool",
                (void *)&uiMsgStorage[0],
                (OS_MEM_QTY)UI_MSG_POOL_SIZE,
                (OS_MEM_SIZE)sizeof(UI_Message_t),
                &err);

    (void)err;
}

UI_Message_t * MsgPool_Alloc(OS_ERR *p_err)
{
    void *p = OSMemGet(&uiMsgMem, p_err);
    if (p != NULL) {
        memset(p, 0, sizeof(UI_Message_t));
    }
    return (UI_Message_t*)p;
}

void MsgPool_Free(UI_Message_t *p_msg, OS_ERR *p_err)
{
    OSMemPut(&uiMsgMem, (void*)p_msg, p_err);
}
