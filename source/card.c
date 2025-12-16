/***************************************************************************//**
@file     card.c
@brief    Driver for card reader RTOS (integrado con message queue)
******************************************************************************/

#include "gpio.h"
#include "card.h"
#include "dneboard.h"
#include <stdbool.h>
#include "msg_pool.h"
#include "ui.h"

/* tus defines, estructuras y variables igual que antes */
enum {WAIT_INIT, WAIT_DATA, WAIT_END, IDLE, VALIDATE_DATA, NO_ACTION};
#define MASK_5bits 0b00011111
#define MAX_ARRAY_SIZE 50
#define MAX_BIT_SIZE 300
#define CARD_SLEEP_TIME 50

/* estructuras internas igual que las tenías */
typedef struct {
    uint8_t byte_data;
    uint16_t bit_counter;
    bool current_bit;
} DataStruct;

typedef struct{
    bool data_value;
    bool clk_value;
    bool ena_value;
} port_data;

OS_TCB  CardTaskTCB;
CPU_STK CardTaskStk[CARD_TASK_STACKSIZE];

static bool f_end_symbol= false;
static bool first_one = false;
static DataStruct myData = {0, 0, 0};
static port_data card_ports = {0,0,0};
static char card_data[MAX_ARRAY_SIZE] = {0};
static char card_data_export[MAX_ID_SIZE] = {0};
bool bit_data[MAX_BIT_SIZE] = {0};
bool CARD_ERROR = false;
bool TEMP_valid_data = false;

/* protos locales */
char byte2char(uint8_t byte_data);
uint8_t ena_check(void);
bool check_init_symbol(void);
char save_char(uint16_t);
bool save_bit_data(void);
void save_data_to_export(void);
void reset_card_functions(void);
void card_IRQ_Handler(void);

/* Provee acceso al ID leído */
char* get_card_id(void)
{
    return (char*)card_data_export;
}

void card_init (void)
{
	gpioMode(PORT_DATA, INPUT);
	gpioMode(PORT_CLOCK, INPUT);
	gpioMode(PORT_ENABLE, INPUT);
	gpioIRQ(PORT_CLOCK, GPIO_IRQ_FALLING_EDGE, card_IRQ_Handler);
}

void Card_TaskCreate(void){
    OS_ERR err;

    OSTaskCreate(&CardTaskTCB,
                 "Card Task",
                 CardTask,
                 0,
                 CARD_TASK_PRIO,
                 &CardTaskStk[0],
				 CARD_TASK_STACKSIZE / 10,
                 CARD_TASK_STACKSIZE,
                 0,
                 0,
                 0,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);
}

void CardTask(void *p_arg){
	(void)p_arg;
	OS_ERR err;
    while (1) {
        /* leemos enable y determinamos si hay que validar */
        card_ports.ena_value = gpioRead(PORT_ENABLE);
        switch (ena_check())
        {
        case NO_ACTION:{
            break;
        }

        case VALIDATE_DATA:{
            uint8_t char_counter = 0;
            if(check_init_symbol() == true){
                for(uint8_t i = 5; (i < (MAX_BIT_SIZE-4) && (f_end_symbol != true) && ((i+4)<(myData.bit_counter))); (i += 5)){
                    if(char_counter < MAX_ARRAY_SIZE){
                        myData.byte_data = save_char(i);
                        switch(myData.byte_data){
                            case '?':
                                f_end_symbol = true;
                                card_data[char_counter] = '\0';
                                break;
                            case '=':
                                char_counter++;
                                break;
                            default:
                                card_data[char_counter] = myData.byte_data;
                                char_counter++;
                                break;
                        }
                    } else {
                        CARD_ERROR=true;
                        break;
                    }
                }
                if(f_end_symbol == true){
                    TEMP_valid_data = true;

                    save_data_to_export();
                    reset_card_functions();

                    /* ---- Aquí publicamos el mensaje a la UI ---- */
                    OS_ERR e2;
                    UI_Message_t *pmsg = MsgPool_Alloc(&e2);
                    if (pmsg != NULL) {
                        pmsg->type = UI_EVENT_CARD_READ;
                        /* copiamos el id (aseguramos terminador) */
                        for (int i = 0; i < MAX_ID_SIZE; ++i)
                            pmsg->data.card_id[i] = card_data_export[i];
                        pmsg->data.card_id[MAX_ID_SIZE-1] = '\0';

                        OSQPost(UI_GetQueue(),
                                (void*)pmsg,
                                0u,
                                OS_OPT_POST_FIFO,
                                &err);
                        (void)err;
                    } else {
                        /* pool vacío: drop o contador */
                    }

                } else {
                    TEMP_valid_data = false;
                    reset_card_functions();
                }
            } else {
                TEMP_valid_data = false;
                reset_card_functions();
            }
            break;
        } /* end VALIDATE_DATA */

        default:
            break;
        } /* end switch */

        OSTimeDlyHMSM(0, 0, 0, CARD_SLEEP_TIME, OS_OPT_TIME_HMSM_STRICT, &err);
    } /* end while */
}

/* ISR Handler (idem a tu versión) */
void card_IRQ_Handler(void)
{
  	CPU_SR_ALLOC();
  	CPU_CRITICAL_ENTER();
  	OSIntEnter();
  	CPU_CRITICAL_EXIT();
  	OS_ERR os_err;

    if((save_bit_data()) == true)
    {
        // ok
    }
    else{
        CARD_ERROR=true;
    }

  	OSIntExit();
}

/* Funciones internas (idénticas a las tuyas) */
bool save_bit_data(void)
{
    myData.current_bit = !gpioRead(PORT_DATA);

    if (first_one == true) {
        if (myData.bit_counter < MAX_BIT_SIZE) {
            bit_data[myData.bit_counter] = myData.current_bit;
            myData.bit_counter++;
        } else {
            return false;
        }
    } else {
        if (myData.current_bit) {
            first_one = true;
            bit_data[myData.bit_counter] = myData.current_bit;
            myData.bit_counter++;
        } else {
        }
    }
    return true;
}

char save_char(uint16_t counter){
    uint8_t aux = 0;
    aux = bit_data[counter] | bit_data[counter+1] << 1 | bit_data[counter+2] << 2 | bit_data[counter+3] << 3 | bit_data[counter+4] << 4;
    char char_data = byte2char(aux);
    return char_data;
}

bool check_init_symbol(void){
    uint8_t aux = 0;
    aux = bit_data[0] << 4 | bit_data[1] << 3 | bit_data[2] << 2 | bit_data[3] << 1 | bit_data[4];
    if(aux == 0b00011010){
        return true;
    }
    return false;
}

char byte2char(uint8_t byte_data){
    char char_data = 0;
    switch(byte_data){
        case 0b00010000: char_data = '0'; break;
        case 0b00000001: char_data = '1'; break;
        case 0b00000010: char_data = '2'; break;
        case 0b00010011: char_data = '3'; break;
        case 0b00000100: char_data = '4'; break;
        case 0b00010101: char_data = '5'; break;
        case 0b00010110: char_data = '6'; break;
        case 0b00000111: char_data = '7'; break;
        case 0b00001000: char_data = '8'; break;
        case 0b00011001: char_data = '9'; break;
        case 0b00011010: char_data = ':'; break;
        case 0b00001011: char_data = ';'; break;
        case 0b00011100: char_data = '<'; break;
        case 0b00001101: char_data = '='; break;
        case 0b00001110: char_data = '>'; break;
        case 0b00011111: char_data = '?'; break;
    }
    return char_data;
}

uint8_t ena_check(void){
    static uint8_t state_set = NO_ACTION ;
    static bool ena_present = HIGH;
    static bool ena_past = HIGH;

    ena_present = card_ports.ena_value;

    if (((ena_present == HIGH) && (ena_past == HIGH)) || ((ena_present == LOW) && (ena_past == LOW)) ) {
        state_set= NO_ACTION;
    } else if ((ena_present == LOW) && (ena_past == HIGH )) {
        state_set= NO_ACTION;
    }
    else if((ena_present == HIGH) && (ena_past == LOW )){
        state_set= VALIDATE_DATA;
    }

    ena_past = ena_present;
    return state_set;
}

void save_data_to_export(void) {
    for (int i = 0; i < (MAX_ID_SIZE-1); i++) {
        card_data_export[i] = card_data[i];
    }
    card_data_export[MAX_ID_SIZE-1] = '\0';
}

void reset_card_functions(void) {
    myData.bit_counter = 0;
    myData.current_bit = 0;
    myData.byte_data = 0;
    first_one = false;
    f_end_symbol = false;
}
