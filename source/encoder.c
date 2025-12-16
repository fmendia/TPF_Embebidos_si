/*******************************************************************************
  encoder.c  (modificado para LONGPRESS configurable)
*******************************************************************************/

#include "encoder.h"
#include "dneboard.h"
#include "gpio.h"
#include "msg_pool.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>

/* RTOS Defines */
#define ENCODER_SLEEP_TIME 1    // in ms

/* Definible: tiempo que considera LONGPRESS en ms */
#ifndef LONGPRESS_TIME_MS
#define LONGPRESS_TIME_MS 1000u
#endif

/* Button states */
#define DEBOUNCE_TICKS 40
typedef enum {
   BUTTON_IDLE,
   BUTTON_DEBOUNCE_PRESS,
   BUTTON_PRESSED,
   BUTTON_DEBOUNCE_RELEASE
} ButtonState;

typedef struct {
   ButtonState state;
   bool event;              // True si se presionó (una sola vez) al soltar
   bool isPressed;          // True mientras esté presionado
   uint32_t pressStart;     // Tiempo de inicio (no usado como timestamp absoluto aquí)
   uint32_t pressDuration;  // Duración acumulada en ms
} Button;

/* RTOS task TCB/stack */
OS_TCB  EncoderTaskTCB;
CPU_STK EncoderTaskStk[ENCODER_TASK_STACKSIZE];

/* Local variables */
int32_t turns = 0; // contador de vueltas desde último read
static bool pin_a, pin_b; // lectura de pines encoder
static Button encoderButton;

/* track if we've already sent a longpress for the current press */
static bool longPressSent = false;

/* protos */
void turn_detect(void);
static void button_status (Button *btn, bool rawInput);

/* inicialización hardware */
void Encoder_Init (void){
    gpioMode(RCHA, INPUT);
    gpioMode(RCHB, INPUT);
    gpioMode(R_SWITCH, INPUT);

    memset(&encoderButton, 0, sizeof(encoderButton));
}

/* task: muestrea, detecta cambios y postea mensajes (encoder/card) */
void EncoderTask(void *p_arg){
    (void)p_arg;
    OS_ERR err;

    while (1) {
        /* muestreo */
        pin_a = gpioRead(RCHA);
        pin_b = gpioRead(RCHB);
        button_status(&encoderButton, gpioRead(R_SWITCH));
        turn_detect();

        /* 1) movimiento */
        if (turns != 0) {
            int32_t delta = encoder_turns(); /* limpia turns */
            OS_ERR e2;
            UI_Message_t *pmsg = MsgPool_Alloc(&e2);
            if (pmsg != NULL) {
                pmsg->type = UI_EVENT_ENCODER_MOVE;
                pmsg->data.encoder_delta = delta;
                OSQPost(UI_GetQueue(), (void*)pmsg, 0u, OS_OPT_POST_FIFO, &err);
                (void)err;
            }
        }

        /* 2) button short press event (Encoder_ButtonWasPressed returns and clears event) */
        if (Encoder_ButtonWasPressed()) {
            OS_ERR e2;
            UI_Message_t *pmsg = MsgPool_Alloc(&e2);
            if (pmsg != NULL) {
                pmsg->type = UI_EVENT_ENCODER_BUTTON;
                pmsg->data.button_pressed = true;
                OSQPost(UI_GetQueue(), (void*)pmsg, 0u, OS_OPT_POST_FIFO, &err);
                (void)err;
            }
        }

        /* 3) long press detection: if button is pressed and pressDuration >= LONGPRESS_TIME_MS
              and we haven't sent longpress yet, then post LONGPRESS event.
           This uses encoderButton.pressDuration which is incremented in button_status()
        */
        if (Encoder_ButtonIsPressed()) {
            if (!longPressSent && Encoder_ButtonGetPressDuration() >= LONGPRESS_TIME_MS) {
                OS_ERR e2;
                UI_Message_t *pmsg = MsgPool_Alloc(&e2);
                if (pmsg != NULL) {
                    pmsg->type = UI_EVENT_ENCODER_LONGPRESS;
                    OSQPost(UI_GetQueue(), (void*)pmsg, 0u, OS_OPT_POST_FIFO, &err);
                    (void)err;
                    longPressSent = true;
                }
            }
        } else {
            /* reset longPressSent when button released */
            longPressSent = false;
        }

        /* dormir */
        OSTimeDlyHMSM(0,0,0,ENCODER_SLEEP_TIME, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

void Encoder_TaskCreate(void){
    OS_ERR err;

    OSTaskCreate(&EncoderTaskTCB,
                 "Encoder Task",
                 EncoderTask,
                 0,
                 ENCODER_TASK_PRIO,
                 &EncoderTaskStk[0],
                 ENCODER_TASK_STACKSIZE / 10,
                 ENCODER_TASK_STACKSIZE,
                 0,
                 0,
                 0,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);
}

/* helpers restantes - no tocadas lógicamente */
int32_t encoder_turns(){
    int32_t temp = turns;
    turns = 0;
    return temp;
}

bool Encoder_ButtonWasPressed(void) {
    if (encoderButton.event) {
        encoderButton.event = false;
        return true;
    }
    return false;
}

bool Encoder_ButtonIsPressed(void) {
    return encoderButton.isPressed;
}

uint32_t Encoder_ButtonGetPressDuration(void) {
    return encoderButton.pressDuration;
}

/* ====================================================================== */
/* turn_detect() - tu algoritmo de quadrature/decodificación (mantengo intacto) */
void turn_detect(){
    static uint16_t historic_state = 0x0000;
    static uint8_t current_state = 0x0000;

    current_state = (pin_a << 1) | pin_b;

    switch(historic_state){
        case 0x0000:
            switch(current_state){
                case 0x0000:
                case 0x0001:
                case 0x0002:
                    historic_state = 0x0000;
                    break;
                case 0x0003:
                    historic_state = 0x0300;
                    break;
            }
            break;
        case 0x0300:
            switch(current_state){
                case 0x0000:
                    historic_state = 0x0000;
                    break;
                case 0x0001:
                case 0x0002:
                    historic_state |= (current_state<<6);
                    break;
                case 0x0003:
                    break;
            }
            break;
        case 0x0380:
        case 0x0340:
            switch(current_state){
                case 0x0000:
                    historic_state |= 0b000000110000;
                    break;
                case 0x0001:
                case 0x0002:
                    if((historic_state & 0b000011000000) != (current_state<<6)){
                        historic_state = 0x0000;
                    }
                    break;
                case 0x0003:
                    historic_state = 0x0300;
                    break;
            }
            break;
        case 0x03B0:
        case 0x0370:
            switch(current_state){
                case 0x0000:
                    break;
                case 0x0001:
                case 0x0002:
                    if((historic_state & 0b000011000000) != (current_state<<6)){
                        historic_state |= (current_state<<2);
                    }
                    break;
                case 0x0003:
                    historic_state = 0x0300;
                    break;
            }
            break;
        case 0x03B4:
        case 0x0378:
            switch(current_state){
                case 0x0000:
                    break;
                case 0x0001:
                case 0x0002:
                    if((historic_state & 0b000000001100) != (current_state<<2)){
                        historic_state = 0x0000;
                    }
                    break;
                case 0x0003:
                    historic_state |= 0b000000000011;
                    break;
            }
            break;
        case 0x03B7:
            turns--;
            historic_state = 0x0300;
            break;
        case 0x037B:
            turns++;
            historic_state = 0x0300;
            break;
    }
}

/* ====================================================================== */
/* button_status() - debounce y press duration (MODIFIED only to use pressDuration ms) */
static void button_status (Button *btn, bool rawInput)
{
    static uint8_t debounceCounter = 0;
    switch (btn->state) {
        case BUTTON_IDLE:
            if (rawInput == 0) { // activo en bajo
                btn->state = BUTTON_DEBOUNCE_PRESS;
                debounceCounter = 0;
            }
            break;

        case BUTTON_DEBOUNCE_PRESS:
            if (rawInput == 0) {
                if (++debounceCounter >= DEBOUNCE_TICKS) {
                    btn->state = BUTTON_PRESSED;
                    btn->event = false;
                    btn->isPressed = true;
                    btn->pressStart = 0;
                    btn->pressDuration = 0;
                }
            } else {
                btn->state = BUTTON_IDLE;
            }
            break;

        case BUTTON_PRESSED:
            if (rawInput == 1) {
                btn->state = BUTTON_DEBOUNCE_RELEASE;
                debounceCounter = 0;
            } else {
                /* acumular duración en ms */
                if (btn->pressDuration < 0xFFFFFFFFu) btn->pressDuration += ENCODER_SLEEP_TIME;
            }
            break;

        case BUTTON_DEBOUNCE_RELEASE:
            if (rawInput == 1) {
                if (++debounceCounter >= DEBOUNCE_TICKS) {

                    btn->state = BUTTON_IDLE;
                    btn->isPressed = false;

                    /*
                     * FIX: No generar short-press si la duración fue >= LONGPRESS_TIME_MS.
                     * Esto evita que al soltar después de un long-press, el sistema envíe un click.
                     */
                    if (btn->pressDuration < LONGPRESS_TIME_MS) {
                        btn->event = true;      // short press
                    } else {
                        btn->event = false;     // bloquear short press
                    }

                    btn->pressDuration = 0;
                }
            } else {
                btn->state = BUTTON_PRESSED;
            }
            break;

            break;
    }
}
