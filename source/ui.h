#ifndef UI_H
#define UI_H

#include <os.h>
#include "msg_pool.h"

/* Inicializa UI (cola + pool + tarea) */
void UI_Init(void);

/* Obtener la cola (para postear con OSQPost si se prefiere) */
OS_Q * UI_GetQueue(void);

#endif /* UI_H */
