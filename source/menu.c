/*
 * menu.c
 *
 * Máquina de estados del menú:
 *  - Main: ID / BRIGHTNESS / CHANGE PIN
 *  - Brightness: 1..5 -> display_brightness(n)
 *  - ID input: 8 dígitos, cursor titilante, backspace por LONGPRESS
 *  - PIN input: longitud 4 o 5, permite 'E' (ENTER) como confirm
 *  - Change PIN flow: ID -> old PIN -> new PIN length -> new PIN -> save
 *
 */

#include "menu.h"
#include "database.h"      
#include "display_rtos.h"
#include "leds.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "display_matrix.h"

#define MENU_CURSOR_BLINK_MS   500u
#define WELCOME_MS             2000u
#define DONE_MS                1500u
#define SCROLL_MESSAGE_MS     600u

#define ID_LEN                 DB_ID_LENGTH
#define DISPLAY_DIGITS         4
#define PIN_MIN_LEN            4
#define PIN_LEN_MAX            5

/* Estados del menú */
typedef enum {
    S_MAIN_MENU = 0,
    S_BRIGHTNESS,
    S_ID_INPUT,
    S_PIN_INPUT,
    S_WELCOME,
    S_ERROR,
    S_CHANGE_PIN_ID,
    S_CHANGE_PIN_OLDPIN,
    S_CHANGE_PIN_LETNEWLEN,
    S_CHANGE_PIN_NEWPIN,
    S_DONE,
    S_NEW,
    S_MENU_ADMIN,
    S_ADD_USER_ID,
    S_ADD_USER_PIN,
    S_DELETE_USER
} menu_state_t;

/* Contexto del menú: todos los datos necesarios */
typedef struct {
    menu_state_t state;

    int main_index;         // 0=ID,1=BRIG,2=CHANGE PIN 
    int menu_admin_index;  // 0=ADD USER,1=DELETE USER

    /* ID buffer */
    char id_buf[DB_ID_LENGTH + 1];
    int id_cursor;          // 0..7, leftmost is 0 
    int window_start;       // starting index for 4-char window 
    bool cursor_on;

    /* PIN buffer */
    char pin_buf[6];        // used for entered pin or new pin; null-terminated 
    int pin_cursor;
    uint8_t expected_pin_len; // expected len for validation (4 or 5)  (no se usa mas)

    int pin_attempts;

    /* change pin specifics */
    uint8_t new_pin_len_choice; //4 or 5 digit long pin
    char new_pin_buf[6];
    int new_pin_cursor;

    /* timers */
    uint32_t blink_ms_acc; //Blink in selection
    uint32_t state_ms_acc;

    char scroll_msg[16]; //For scrolling messages

    uint16_t idx; //Index of the user being processed
} menu_menu_context_t;

static menu_menu_context_t menu_context;
static char id_edit = '0';
static char pin_edit = '0';


//Prototipos de funciones internas

//Renders: Mostrar en display segun el estado actual
static void render_main_menu(void);
static void render_brightness(int bri);
static void render_id_input(void);
static void render_pin_input(void);
static void render_menu_admin(void);



//Funciones para entrar a los distintos estados, cambian variables de contexto y hacen render inicial
static void enter_main_menu(void);
static void enter_brightness(void);
static void enter_id_input_state(void);
static void enter_pin_input_for_user(int user_idx);
static void enter_change_pin_id(void);
static void enter_menu_admin(void);

//Input handling
static void handle_encoder_move_int(int32_t delta);
static void handle_button_shortpress (void);
static void handle_longpress_backspace(void);

//Funciones para procesar id y pin ingresados
static void process_entered_pin_for_user(void);
static void process_entered_new_pin_for_user(void);
static void process_entered_id_for_user(void);

//Funciones auxiliares
static void main_move(int delta);
static void menu_admin_move(int delta);
char select_char_with_encoder(int32_t delta);

/* render main menu option */
static const char *main_texts[3] = { "ID  ", "BRIG", "PIN " };

/************************** Functions *************************************************************************** */
/* Initialization */
void Menu_Init(void)
{
    DB_Init();  /* NEW: initialize database */

    memset(&menu_context, 0, sizeof(menu_context));
    menu_context.state = S_MAIN_MENU;
    menu_context.main_index = 0;
    menu_context.menu_admin_index = 0;
    menu_context.cursor_on = true;
    menu_context.window_start = 0;
    menu_context.blink_ms_acc = 0;
    menu_context.state_ms_acc = 0;
    render_main_menu();
}

/* ------------------ External message handler ------------------ */
//Este es la posta. Maneja los mensajes entrantes y llama a el handler correspondiente segun el tipo de evento.
void Menu_HandleMessage(UI_Message_t *pmsg)
{
    if (!pmsg) return;

    switch (pmsg->type) {
        case UI_EVENT_ENCODER_MOVE:
            handle_encoder_move_int(pmsg->data.encoder_delta);
            break;
        case UI_EVENT_ENCODER_BUTTON:
            handle_button_shortpress();
            break;
        case UI_EVENT_ENCODER_LONGPRESS:
            /* backspace semantics */
            handle_longpress_backspace();
            break;
        case UI_EVENT_CARD_READ:
            /* copy up to ID_LEN, validate immediately */
            memset(menu_context.id_buf, 0, sizeof(menu_context.id_buf));
            for (int i = 0; i < ID_LEN; ++i) {
                menu_context.id_buf[i] = pmsg->data.card_id[i] ? pmsg->data.card_id[i] : '0';
            }
            menu_context.id_buf[ID_LEN] = '\0';
            {
                int idx = DB_FindUserById(menu_context.id_buf);
                if (idx >= 0)
                {
                    if (DB_IsAdmin(idx))
                    {
                        menu_context.state = S_MENU_ADMIN;
                       enter_menu_admin();
                    }
                    else
                    {
                        menu_context.state = S_ID_INPUT;
                    enter_pin_input_for_user(idx);
                    }
                }
                else { menu_context.state = S_ERROR; menu_context.state_ms_acc = 0; display_string4("ERR "); }
            }
            break;
        default:
            break;
    }
}

/* ------------------ Tick handling ------------------ */

/* Menu_Tick called on timeout.
   We accumulate time and toggle cursor every MENU_CURSOR_BLINK_MS.
   Para hacer los renders necesarios cuando no se ingresa ningun msg*/
void Menu_Tick(void)
{
    menu_context.blink_ms_acc += 200u;
    if (menu_context.blink_ms_acc >= MENU_CURSOR_BLINK_MS) {
        menu_context.blink_ms_acc = 0;
        menu_context.cursor_on = !menu_context.cursor_on;

        /* re-render visible state for blink */
        switch (menu_context.state) {
            case S_MAIN_MENU: render_main_menu(); break;
            case S_BRIGHTNESS: break; /* static */
            case S_ID_INPUT: render_id_input(); break;
            case S_PIN_INPUT: render_pin_input(); break;
            case S_CHANGE_PIN_ID: render_id_input(); break;
            case S_CHANGE_PIN_OLDPIN: render_pin_input(); break;
            case S_CHANGE_PIN_NEWPIN: render_pin_input(); break;
            case S_MENU_ADMIN: render_menu_admin(); break;
            case S_ADD_USER_ID: render_id_input(); break;
            case S_ADD_USER_PIN: render_pin_input(); break;
            case S_WELCOME:
                menu_context.state_ms_acc += MENU_CURSOR_BLINK_MS;
                if (menu_context.state_ms_acc >= WELCOME_MS) {
                    menu_context.state = S_MAIN_MENU;
                    render_main_menu();
                }
                break;
            case S_DONE:
                menu_context.state_ms_acc += MENU_CURSOR_BLINK_MS;
                if (menu_context.state_ms_acc >= DONE_MS) {
                    menu_context.state = S_MAIN_MENU;
                    render_main_menu();
                }
                break;
            case S_ERROR:
                menu_context.state_ms_acc += MENU_CURSOR_BLINK_MS;
                if (menu_context.state_ms_acc >= MENU_CURSOR_BLINK_MS * 2) {
                    menu_context.state = S_MAIN_MENU;
                    render_main_menu();
                }
                break;
            default:
                break;
        }
    }
    else if(menu_context.blink_ms_acc >= SCROLL_MESSAGE_MS)
    {
        // Additional handling for scrolling messages can be added here
        ScrollMessage(menu_context.scroll_msg);
    }
}

/* Force show home */
void Menu_ShowHome(void)
{
    menu_context.state = S_MAIN_MENU;
    render_main_menu();
}
/* ------------------ Render functions ------------------ */

static void render_main_menu(void)
{
    if (menu_context.main_index < 2) {
        display_string4(main_texts[menu_context.main_index]);
    }
    else {
       strcpy(menu_context.scroll_msg, "CHG PIN   "); 
       ScrollMessage(menu_context.scroll_msg);
    }
    
}
static void render_menu_admin(void)
{
    if (menu_context.menu_admin_index == 0) {
        display_string4("ADD ");
    }
    else {
       display_string4("DEL ");
    }

}
/* render brightness preview: show 'B' + digit */
static void render_brightness(int bri)
{
    char t[5] = { ' ', 'B', (char)('0' + bri), ' ' };
    display_string4(t);
}
/* render ID input: blinking cursor */
void render_id_input(void)
{
    // Display editable digit at position 0 (rightmost) - blinking
    if (menu_context.cursor_on) {
        display_char(id_edit, 0);
    } else {
        display_char(' ', 0);  // blink: show space when cursor_on is false
    }

    // Display previously confirmed digits at positions 1-3
    for (uint16_t i = 1; i < NUMBER_DISPLAYS; i++)
    {
        if (menu_context.id_cursor - i >= 0)
        {
            display_char(menu_context.id_buf[menu_context.id_cursor - i], i);
        }
        else
        {
            display_char('-', i);
        }
    }
}

/* render PIN input: */
static void render_pin_input(void)
{
    // Display editable digit at position 0 (rightmost) - blinking
    if (menu_context.cursor_on) {
        display_char(pin_edit, 0);
    } else {
        display_char(' ', 0);  // blink: show space when cursor_on is false
    }

    // Display previously confirmed digits at positions 1-3
    for (uint16_t i = 1; i < NUMBER_DISPLAYS; i++)
    {
        if (menu_context.pin_cursor - i >= 0)
        {
            display_char('-', i);  // show dash instead of actual PIN for security
        }
        else
        {
            display_char(' ', i); //empty, podriamos moner otro caracter
        }
    }
}
/* ------------------ State changes ------------------ */
//Como entrar a los distintos estados
static void enter_main_menu(void)
{
    menu_context.state = S_MAIN_MENU;
    menu_context.main_index = 0;
    render_main_menu();
}

static void enter_brightness(void)
{
    menu_context.state = S_BRIGHTNESS;
    /* default preview 3 */
    render_brightness(3);
}

static void enter_id_input_state(void)
{
    menu_context.state = S_ID_INPUT;
    memset(menu_context.id_buf, 0, sizeof(menu_context.id_buf));//borro buffer
    menu_context.id_cursor = 0;
    menu_context.window_start = 0;
    menu_context.cursor_on = true;
    menu_context.blink_ms_acc = 0;
    render_id_input();
}

static void enter_pin_input_for_user(int user_idx)
{
    if (menu_context.state== S_ID_INPUT)
    {
        menu_context.state = S_PIN_INPUT;
    }
    else if( menu_context.state == S_CHANGE_PIN_ID)
    {
        menu_context.state = S_CHANGE_PIN_OLDPIN;
    }

    menu_context.pin_attempts = 0;
    //menu_context.expected_pin_len = user_db[user_idx].pin_len;
    menu_context.expected_pin_len = DB_GetUserPinLength(user_idx);
    memset(menu_context.pin_buf, 0, sizeof(menu_context.pin_buf));
    menu_context.pin_cursor = 0;
    menu_context.window_start = 0;
    menu_context.cursor_on = true;
      menu_context.idx = user_idx;
    render_pin_input();
}

static void enter_change_pin_id(void)
{
    menu_context.state = S_CHANGE_PIN_ID;
    memset(menu_context.id_buf, 0, sizeof(menu_context.id_buf));
    menu_context.id_cursor = 0;
    menu_context.window_start = 0;
    menu_context.cursor_on = true;
    render_id_input();
}

static void enter_menu_admin(void)
{
    menu_context.state = S_MENU_ADMIN;
    display_string4("ADM ");
    render_menu_admin();
}


/* ------------------ Input handling ------------------ */

//Segun en que estado estoy, hago lo correspondiente al mover el encoder
static void handle_encoder_move_int(int32_t delta)
{
    if (delta == 0) return;

    if (menu_context.state == S_MAIN_MENU)
    {
        main_move(delta > 0 ? 1 : -1);//para no mover de a mas de 1 posicion
    }
    else if(menu_context.state == S_MENU_ADMIN)
    {
        menu_admin_move(delta > 0 ? 1 : -1);
    }
    else if (menu_context.state == S_BRIGHTNESS)
    {
        static int bri = 3;
        bri += (delta > 0) ? 1 : -1;
        if (bri < 1) bri = 1;
        if (bri > 5) bri = 5;
        render_brightness(bri);
        display_brightness(bri);
    }
    else if (menu_context.state == S_ID_INPUT || menu_context.state == S_CHANGE_PIN_ID || menu_context.state == S_ADD_USER_ID || menu_context.state == S_DELETE_USER)
    {
        /* numeric digits 0-9 + E (ENTER) */
        char c = select_char_with_encoder(delta);
        id_edit = c;
        render_id_input();
    }
    else if (menu_context.state == S_PIN_INPUT || menu_context.state == S_CHANGE_PIN_OLDPIN || menu_context.state == S_CHANGE_PIN_NEWPIN || menu_context.state == S_ADD_USER_PIN)
    {
        /* numeric digits 0-9 + E (ENTER) */
        char c = select_char_with_encoder(delta);
        pin_edit = c;
        render_pin_input();
    }
}
//Una vez apretado enter 'E', proceso el pin ingresado segun en que estado estoy.
static void process_entered_pin_for_user(void)
{
    if (menu_context.pin_cursor >= PIN_MIN_LEN && menu_context.pin_cursor <= PIN_LEN_MAX) //Si se ingrso un pin del tamano correcto
    {
        if (DB_VerifyPin(menu_context.idx, menu_context.pin_buf, menu_context.pin_cursor)) //Si esta el pin
        {
            /* success */
            if(menu_context.state == S_PIN_INPUT) //Welcome si estoty en flujo normal
            {
            	uint8_t piso = DB_GetUserFloor(menu_context.idx);
                if (DB_IsUserInBuilding(menu_context.idx))
                {

                    //Si el usuario ya esta en el edificio, lo saco
                    menu_context.state = S_DONE;
                    menu_context.state_ms_acc = 0;
                    display_string4("BYE ");
                    pin_edit = '0';//reset editable char
                    id_edit = '0';
                    Matrix_RemovePerson(piso); //Saco persona del piso 1
                    DB_SetUserInBuilding(menu_context.idx,false);
                }
                else
                {
                    menu_context.pin_attempts = 0;
                    menu_context.state = S_WELCOME;
                    menu_context.state_ms_acc = 0;
                    display_string4("WELC");
                    pin_edit = '0';//reset editable char
                    id_edit = '0';
                    Matrix_AddPerson(piso); //Agrego persona al piso 1
                    DB_SetUserInBuilding(menu_context.idx,true);
                    
                }
            } 
            else if (menu_context.state == S_CHANGE_PIN_OLDPIN) 
            {//si estoy en cambio de pin y el pin viejo es correcto paso a elegir el nuevo pin
                menu_context.state = S_CHANGE_PIN_NEWPIN;
                display_string4("NEW ");
                memset(menu_context.pin_buf, 0, sizeof(menu_context.pin_buf));
                menu_context.pin_cursor = 0;
                menu_context.window_start = 0;
                menu_context.cursor_on = true;
                render_pin_input();
            }
        } 
        else 
        {//Pin incorrecto. Para ambos casos, si se equivoca el pin 3 veces borra el usuario.
                menu_context.pin_attempts++;
                if (menu_context.pin_attempts >= 3) {
                    /* NEW: use DB_DeleteUser */
                    DB_DeleteUser(menu_context.idx);
                    menu_context.state = S_ERROR;
                    menu_context.state_ms_acc = 0;
                    display_string4("DLT ");
                } else {
                    menu_context.state_ms_acc = 0;
                    display_string4("MAL ");
                    memcpy(menu_context.pin_buf, 0, sizeof(menu_context.pin_buf));
                    menu_context.pin_cursor = 0;
                    pin_edit = '0';
                    id_edit = '0';
                }
        }
    } 
    else 
    {
        /* PIN length out of range */
        display_string4("RNGE");
       // menu_context.state = S_ERROR;
        memcpy(menu_context.pin_buf, 0, sizeof(menu_context.pin_buf));
        menu_context.pin_cursor = 0;
        pin_edit = '0';
        id_edit = '0';
    }
}

//Es similar a la funcion anterior, pero para el nuevo pin. Las separe por claridad, me estaba re mareando xd
static void process_entered_new_pin_for_user(void)
{
     if (menu_context.pin_cursor >= PIN_MIN_LEN && menu_context.pin_cursor <= PIN_LEN_MAX) //Si se ingrso un pin del tamano correcto
    {
        DB_ChangeUserPin(menu_context.idx, menu_context.pin_buf, menu_context.pin_cursor);//Esta funcion esta mal, onda hay que ver como cambiar bien el pin y que funcione.
        menu_context.state = S_DONE;
        menu_context.state_ms_acc = 0;
        display_string4("DONE");
    } 
    else 
    {
        /* PIN length out of range */
        display_string4("RNGE");
        //menu_context.state = S_ERROR;
        memcpy(menu_context.pin_buf, 0, sizeof(menu_context.pin_buf));
        menu_context.pin_cursor = 0;
        pin_edit = '0';
        id_edit = '0';
    }
}

//Si aprete ender, proceso el id ingresado segun en que estado estoy.
static void process_entered_id_for_user(void)
{
    if (menu_context.id_cursor > 0)//Si se ingresaron digitos
    {
        //busco en data base si esta ese id
        int idx = DB_FindUserById(menu_context.id_buf);
        if (idx >= 0) 
        {//si esta, entro a pedir pin
            enter_pin_input_for_user(idx);
        } else {
            //si no esta, muestro error (error vuelve a main menu)
           display_string4("ERR ");
            menu_context.state = S_ERROR;
            menu_context.state_ms_acc = 0;
        }
    } else {
        /* no digits entered yet */
        display_string4("ERR ");
        menu_context.state = S_ERROR;
        menu_context.state_ms_acc = 0;
    }
    return;
}

//Handler para el apretado corto del boton: segun en que estado estoy hago lo correspondiente
static void handle_button_shortpress (void)
{
    switch (menu_context.state)
    {
    case S_MAIN_MENU://Selecciono la opcion del menu principal
       if (menu_context.main_index == 0) enter_id_input_state();
        else if (menu_context.main_index == 1) enter_brightness();
        else enter_change_pin_id();
        break;
    case S_BRIGHTNESS://Confirmo el cambio de brillo y vuelvo al menu principal
        menu_context.state = S_MAIN_MENU;
        render_main_menu();
        break;
    case S_MENU_ADMIN://Selecciono la opcion del menu admin
        if (menu_context.menu_admin_index == 0) //ADD USER
        {
            menu_context.state = S_ADD_USER_ID;
            memset(menu_context.id_buf, 0, sizeof(menu_context.id_buf));//borro buffer
            menu_context.id_cursor = 0;
            menu_context.window_start = 0;
            menu_context.cursor_on = true;
            menu_context.blink_ms_acc = 0;
            render_id_input();
        }
        else //DELETE USER
        {
            menu_context.state = S_DELETE_USER;
            memset(menu_context.id_buf, 0, sizeof(menu_context.id_buf));//borro buffer
            menu_context.id_cursor = 0;
            menu_context.window_start = 0;
            menu_context.cursor_on = true;
            menu_context.blink_ms_acc = 0;
            render_id_input();
        }
        break;
    case S_ID_INPUT://Proceso el digito ingresado, al ingresar un id
    case S_CHANGE_PIN_ID://Para ambos casos es igual el manejo
        if (id_edit == 'E') //Enter pressed
        {
            process_entered_id_for_user();
        } 
        else 
        {//Estoy ingresando digitos
            if (menu_context.id_cursor < ID_LEN) {
                menu_context.id_buf[menu_context.id_cursor++] = id_edit;
                menu_context.id_buf[menu_context.id_cursor] = '\0';
                //id_edit = '0';
                render_id_input();
            }
        }
        break;
    case S_DELETE_USER://Proceso el id ingresado para borrar usuario
        if (id_edit == 'E') //Enter pressed
        {
            int idx = DB_FindUserById(menu_context.id_buf);
            if (idx >= 0)
            {
                if (DB_IsAdmin(idx))
                {
                    display_string4("ADM ");
                    menu_context.state = S_ERROR;
                    menu_context.state_ms_acc = 0;
                    return;
                }
                else{
                    DB_DeleteUser(idx);
                    menu_context.state = S_DONE;
                    menu_context.state_ms_acc = 0;
                    display_string4("DLT ");
                }

            }
            else
            {
                display_string4("ERR ");
                menu_context.state = S_ERROR;
                menu_context.state_ms_acc = 0;
            }
        }
        else
        {//Estoy ingresando digitos
            if (menu_context.id_cursor < ID_LEN) {
                menu_context.id_buf[menu_context.id_cursor++] = id_edit;
                menu_context.id_buf[menu_context.id_cursor] = '\0';
                //id_edit = '0';
                render_id_input();
            }
        }
        break;
    case S_ADD_USER_ID://Proceso el id ingresado para agregar usuario
        if (id_edit == 'E') //Enter pressed
        {
            int idx = DB_FindUserById(menu_context.id_buf);
            if (idx < 0)
            { //Si no existe el usuario
                if (DB_FindEmptySlot() < 0) //No hay lugar para mas usuarios
                {
                    display_string4("FULL");
                    menu_context.state = S_ERROR;
                    menu_context.state_ms_acc = 0;
                    return;
                }
                else{
                    //Hay lugar, sigo con el flujo
                    menu_context.state = S_ADD_USER_PIN;
                    memset(menu_context.pin_buf, 0, sizeof(menu_context.pin_buf));
                    menu_context.pin_cursor = 0;
                    menu_context.window_start = 0;
                    menu_context.cursor_on = true;
                    render_pin_input();
                }
            }
            else
            {
                display_string4("EXST");
                menu_context.state = S_ERROR;
                menu_context.state_ms_acc = 0;
            }
        }
        else
        {//Estoy ingresando digitos
            if (menu_context.id_cursor < ID_LEN) {
                menu_context.id_buf[menu_context.id_cursor++] = id_edit;
                menu_context.id_buf[menu_context.id_cursor] = '\0';
                //id_edit = '0';
                render_id_input();
            }
        }
    case S_PIN_INPUT://Proceso el digito ingresado, al ingresar un pin
    case S_CHANGE_PIN_OLDPIN://Es igual el manejo, lo importante es verificar pin del usuario
        if (pin_edit == 'E') {
            process_entered_pin_for_user();
        } 
        else {
            /* regular digit: append to pin_full */
            if (menu_context.pin_cursor < PIN_LEN_MAX) {
                menu_context.pin_buf[menu_context.pin_cursor++] = pin_edit;
                menu_context.pin_buf[menu_context.pin_cursor] = '\0';
               // pin_edit = '0';
                render_pin_input();
            }
        }
        break;
    case S_CHANGE_PIN_NEWPIN://Para poner el nuevo pin, es parecido al caso anterior solo que si aprieto enter guardo el nuevo pin(separado para entender bien jaja)
        if (pin_edit == 'E') {
            process_entered_new_pin_for_user();
        } 
        else {
            /* regular digit: append to pin_full */
            if (menu_context.pin_cursor < PIN_LEN_MAX) {
                menu_context.pin_buf[menu_context.pin_cursor++] = pin_edit;
                menu_context.pin_buf[menu_context.pin_cursor] = '\0';
                //pin_edit = '0';
                render_pin_input();
            }
        }
        break;
    case S_ADD_USER_PIN://Proceso el pin ingresado para agregar usuario
        if (pin_edit == 'E') //Enter pressed
        {
            if (menu_context.pin_cursor >= PIN_MIN_LEN && menu_context.pin_cursor <= PIN_LEN_MAX) //Si se ingrso un pin del tamano correcto
            {
                DB_AddUser(menu_context.id_buf, menu_context.pin_buf, menu_context.pin_cursor,1); //Agrego usuario como no admin y en piso 1
                menu_context.state = S_DONE;
                menu_context.state_ms_acc = 0;
                display_string4("DONE");
            }
            else
            {
                /* PIN length out of range */
                display_string4("RNGE");
                //menu_context.state = S_ERROR;
                memcpy(menu_context.pin_buf, 0, sizeof(menu_context.pin_buf));
                menu_context.pin_cursor = 0;
                pin_edit = '0';
                id_edit = '0';
            }
        }
        else
        {//Estoy ingresando digitos
            if (menu_context.pin_cursor < PIN_LEN_MAX) {
                menu_context.pin_buf[menu_context.pin_cursor++] = pin_edit;
                menu_context.pin_buf[menu_context.pin_cursor] = '\0';
                //pin_edit = '0';
                render_pin_input();
            }
        }
        break;
    default:
        break;
    }
}

//Backspace handler for long press. Para borrar digitos o volver al menu principal si no hay digitos para borrar
static void handle_longpress_backspace(void)
{
    if (menu_context.state == S_ID_INPUT || menu_context.state == S_CHANGE_PIN_ID) {
        if (menu_context.id_cursor > 0) {
            /* remove last confirmed digit */
            menu_context.id_cursor--;
            menu_context.id_buf[menu_context.id_cursor] = '\0';
            /* keep editable digit as-is (user can reselect) */
            render_id_input();
        } else {
            /* if nothing to delete, go back to main */
            menu_context.state = S_MAIN_MENU;
            render_main_menu();
        }
    } else if (menu_context.state == S_PIN_INPUT || menu_context.state == S_CHANGE_PIN_OLDPIN || menu_context.state == S_CHANGE_PIN_NEWPIN) {
        if (menu_context.pin_cursor > 0) {
            menu_context.pin_cursor--;
            menu_context.pin_buf[menu_context.pin_cursor] = '\0';
            render_pin_input();
        } else {
            menu_context.state = S_MAIN_MENU;
            render_main_menu();
        }
    }
}
/* ------------------ Helper functions ------------------ */
/* Helper to move main selection (wraparound) */
static void main_move(int delta)
{
    menu_context.main_index += delta;
    if (menu_context.main_index < 0) menu_context.main_index = 2;
    if (menu_context.main_index > 2) menu_context.main_index = 0;
    render_main_menu();
}

static void menu_admin_move(int delta)
{
    menu_context.menu_admin_index += delta;
    if (menu_context.menu_admin_index < 0) menu_context.menu_admin_index = 1;
    if (menu_context.menu_admin_index > 1) menu_context.menu_admin_index = 0;
    render_menu_admin();
}

//mismo que en tp1
char select_char_with_encoder(int32_t delta)
{
    static const char char_list[] = "E0123456789";
    static int index = 0;
    int max_index = sizeof(char_list) - 2;//maximo indice es el sizeof menos el terminador y menos 1

    index += delta;
    if (index < 0)
        index = max_index;
    if (index > max_index)
        index = 0;

    return char_list[index];
}

