#ifndef MENU_H
#define MENU_H

#include "msg_pool.h"

/* Inicializa el módulo menu (llamar desde UI_Init) */
void Menu_Init(void);

/* Procesa un mensaje proveniente de la queue (encoder/button/card) */
void Menu_HandleMessage(UI_Message_t *pmsg);

/* Tick periódico (toggle de cursor, timeouts). Llamar cuando OSQPend timeout ocurre. */
void Menu_Tick(void);

/* Forzar mostrar menu principal (opcional) */
void Menu_ShowHome(void);

#endif /* MENU_H */
