/***************************************************************************//**
  @file     display.c
  @brief    Display functions
  @author   Emma Herrera
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

 #include "display_rtos.h"
 #include "dneboard.h"
 #include "gpio.h"
 #include "string.h"
 #include <stdbool.h>
 #include  <os.h>
 #define DEFAULT_BRIGHTNESS 1 // Default brightness level for the display
 /*******************************************************************************
  * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
  ******************************************************************************/
const uint8_t segment_pins[8] = {
    PIN2NUM(SEGA),
    PIN2NUM(SEGB),
    PIN2NUM(SEGC),
    PIN2NUM(SEGD),
    PIN2NUM(SEGE),
    PIN2NUM(SEGF),
    PIN2NUM(SEGG),
    PIN2NUM(SEGDP)
};

 uint32_t seg2port(uint8_t seg);
 uint8_t num27seg(uint8_t num);

 //void num2disp(uint32_t num);
 //void display_select(uint8_t digit);

 void Display_Callback(void);

 uint32_t display_buffer[NUMBER_DISPLAYS] = {0}; // Buffer to hold the display content
uint8_t brightness = DEFAULT_BRIGHTNESS; // Default brightness level
 bool flag = 0;
 static uint32_t mask =0;

 static uint8_t scroll_index = 0;
static OS_MUTEX MyMutex;
 /*******************************************************************************
  *******************************************************************************
                         GLOBAL FUNCTION DEFINITIONS
  *******************************************************************************
  ******************************************************************************/

static void TaskDisplay(void *p_arg){
	(void)p_arg;
	OS_ERR os_err;

	while(1){

		refresh_display(); //Refresh display buffer
		OSTimeDlyHMSM(0u, 0u, 0u, 1u, OS_OPT_TIME_HMSM_STRICT, &os_err);
	}
}

void Display_TaskCreate(void){
    OS_ERR os_err;

    OSTaskCreate(&TaskDisplayTCB, 			//tcb
    				 "Task Display",				//name
    				  TaskDisplay,				//func
    				  0u,					//arg
    				  TASKDISP_PRIO,			//prio
    				 &TaskDisplayStk[0u],			//stack
    				  TASKDISP_STK_SIZE_LIMIT,	//stack limit
    				  TASKDISP_STK_SIZE,		//stack size
    				  0u,
    				  0u,
    				  0u,
    				 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
    				 &os_err);
}


 void Display_Init (void)
 {
	 OS_ERR err;
     gpioMode(STATUS0, OUTPUT);
     gpioMode(STATUS1, OUTPUT);
     gpioMode(SEGA, OUTPUT);
     gpioMode(SEGB, OUTPUT);
     gpioMode(SEGC, OUTPUT);
     gpioMode(SEGD, OUTPUT);
     gpioMode(SEGE, OUTPUT);
     gpioMode(SEGF, OUTPUT);
     gpioMode(SEGG, OUTPUT);
     gpioMode(SEGDP, OUTPUT);
     gpioMode(SEL0, OUTPUT);
     gpioMode(SEL1, OUTPUT);
     OSMutexCreate(&MyMutex, "MyMutex", &err);
     mask = seg2port(0xFF);

     //SysTick_Manager(Display_Callback, 1);
 }

 void display_char(uint8_t digit, uint8_t position){
	OS_ERR os_err;
	CPU_TS ts;

    OSMutexPend((OS_MUTEX *)&MyMutex,(OS_TICK)0, (OS_OPT)OS_OPT_PEND_BLOCKING, (CPU_TS *)&ts, (OS_ERR*)&os_err);
    if (position < NUMBER_DISPLAYS) {

       display_buffer[position] = seg2port(num27seg(digit)); // Store the character in the buffer
    }
    else {
       // Handle error: position out of range
       return;
    }
    OSMutexPost((OS_MUTEX *)&MyMutex, (OS_OPT)OS_OPT_PEND_BLOCKING, (OS_ERR*)&os_err);
}

 void display_string4(const char* str)
 {

		  for (int j = 0; j < NUMBER_DISPLAYS; ++j) {
        int disp_pos = 3 - j; /* leftmost -> pos 3 */
        display_char(str[j], disp_pos);
    }

 }

void clear_display(void){
    OS_ERR os_err;
    CPU_TS ts;

    OSMutexPend((OS_MUTEX *)&MyMutex, (OS_TICK)0, (OS_OPT)OS_OPT_PEND_BLOCKING, (CPU_TS *)&ts, (OS_ERR*)&os_err);
    for (uint8_t i = 0; i < NUMBER_DISPLAYS; i++) {
        display_buffer[i] = seg2port(num27seg(' ')); // Clear the buffer
    }
    OSMutexPost((OS_MUTEX *)&MyMutex, (OS_OPT)OS_OPT_PEND_BLOCKING, (OS_ERR*)&os_err);
}

void clear_digit(uint8_t position){
    OS_ERR os_err;
    CPU_TS ts;

    OSMutexPend((OS_MUTEX *)&MyMutex, (OS_TICK)0, (OS_OPT)OS_OPT_PEND_BLOCKING, (CPU_TS *)&ts, (OS_ERR*)&os_err);
    if (position < NUMBER_DISPLAYS) {
        display_buffer[position] = seg2port(num27seg(' ')); // Clear the specific digit
    }
    else {
        // Handle error: position out of range
        return;
    }
    OSMutexPost((OS_MUTEX *)&MyMutex, (OS_OPT)OS_OPT_PEND_BLOCKING, (OS_ERR*)&os_err);
}

void display_brightness(uint8_t bright_value){
    if (bright_value > VERY_HIGH) {
        bright_value = VERY_HIGH; // Limit the maximum brightness
    }
    else if(bright_value < VERY_LOW){
        bright_value =VERY_LOW; //Limit the minimum brightness
    }
    brightness = bright_value; // Set the brightness level
}


void shift_display(uint8_t direction, uint8_t shift_amount){
    OS_ERR os_err;
    CPU_TS ts;

    OSMutexPend((OS_MUTEX *)&MyMutex, (OS_TICK)0, (OS_OPT)OS_OPT_PEND_BLOCKING, (CPU_TS *)&ts, (OS_ERR*)&os_err);
    if (direction == 0) { // Shift right
        for (uint8_t i = 0; i < NUMBER_DISPLAYS; i++) {
            if (i + shift_amount < NUMBER_DISPLAYS) {
                display_buffer[i] = display_buffer[i + shift_amount];
            } else {
                display_buffer[i] = seg2port(num27seg(' ')); // Clear the shifted positions
            }
        }
    } else { // Shift left
        for (int8_t i = NUMBER_DISPLAYS - 1; i >= 0; i--) {
            if (i - shift_amount >= 0) {
                display_buffer[i] = display_buffer[i - shift_amount];
            } else {
                display_buffer[i] = seg2port(num27seg(' ')); // Clear the shifted positions
            }
        }
    }
    OSMutexPost((OS_MUTEX *)&MyMutex, (OS_OPT)OS_OPT_PEND_BLOCKING, (OS_ERR*)&os_err);
}

void shift_display_reposition(uint8_t direction, char new_digit){
    OS_ERR os_err;
    CPU_TS ts;

    OSMutexPend((OS_MUTEX *)&MyMutex, (OS_TICK)0, (OS_OPT)OS_OPT_PEND_BLOCKING, (CPU_TS *)&ts, (OS_ERR*)&os_err);
    if (direction == 1) { // Shift right
        for (uint8_t i = NUMBER_DISPLAYS - 1; i > 0; i--) {
            display_buffer[i] = display_buffer[i - 1]; // Shift left
        }
        display_buffer[0] = seg2port(num27seg(new_digit)); // Set the new digit at the beginning
    } else { // Shift left
        for (uint8_t i = 0; i < NUMBER_DISPLAYS - 1; i++) {
            display_buffer[i] = display_buffer[i + 1]; // Shift right
        }
        display_buffer[NUMBER_DISPLAYS - 1] = seg2port(num27seg(new_digit)); // Set the new digit at the end
    }
    OSMutexPost((OS_MUTEX *)&MyMutex, (OS_OPT)OS_OPT_PEND_BLOCKING, (OS_ERR*)&os_err);
}


void refresh_display(void){
    static uint8_t digit = 0;
	static uint8_t pwm_counter = 0;
	//static uint8_t brightness = 3;

    OS_ERR os_err;
    CPU_TS ts;

    num2disp(0x0000); //Turn off before changing the selector
		display_select(digit);
		if (pwm_counter < brightness)
		{
			OSMutexPend((OS_MUTEX *)&MyMutex,(OS_TICK)0, (OS_OPT)OS_OPT_PEND_BLOCKING, (CPU_TS *)&ts, (OS_ERR*)&os_err);

			num2disp(display_buffer[digit]);
			//led_select(selected_led); //Only for debug

			OSMutexPost((OS_MUTEX *)&MyMutex,
								(OS_OPT)OS_OPT_PEND_BLOCKING,
								(OS_ERR*)&os_err);
		 }
		 if (++digit >= 4){
			digit = 0; // Reset demux
			pwm_counter = (pwm_counter + 1) % (VERY_HIGH+1); // 5 levels of brightness
		 }
}
void ResetScroll(void)
{
    scroll_index = 0;
}
void ScrollMessage(char *message)
{
    //static uint32_t index = 0;
    static char prev_message[20];
    
    uint32_t length = strlen(message);

    // Si el mensaje cambió, reiniciamos
    if (strcmp(message, prev_message) != 0) {
        scroll_index = 0;
        strncpy(prev_message, message, sizeof(prev_message) - 1);
        prev_message[sizeof(prev_message) - 1] = '\0'; // seguridad
        clear_display();
    }

    // Calcular el largo total a scrollear (mensaje + espacios al final)
    uint32_t total_length = length + NUMBER_DISPLAYS / 2;

    // Selección del caracter a mostrar
    char c = (scroll_index < length) ? message[scroll_index] : ' ';

    shift_display_reposition(1, c);

    // Avanzar índice con wrap-around
    scroll_index = (scroll_index + 1) % total_length;
}



 /*******************************************************************************
  *******************************************************************************
                         LOCAL FUNCTION DEFINITIONS
  *******************************************************************************
  ******************************************************************************/

 uint8_t num27seg(uint8_t num){
     switch(num){
         case ' ':
             return 0b00000000; // All segments off
         case '0':
             return 0b00111111; // Active high representation for 0
         case '1':
             return 0b00000110; // Active high representation for 1
         case '2':
             return 0b01011011; // Active high representation for 2
         case '3':
             return 0b01001111; // Active high representation for 3
         case '4':
             return 0b01100110; // Active high representation for 4
         case '5':
             return 0b01101101; // Active high representation for 5
         case '6':
             return 0b01111101; // Active high representation for 6
         case '7':
             return 0b00000111; // Active high representation for 7
         case '8':
             return 0b01111111; // Active high representation for 8
         case '9':
             return 0b01101111; // Active high representation for 9
         case 'A':
             return 0b01110111; // Active high representation for A
         case 'B':
             return 0b01111100; // Active high representation for B
         case 'C':
             return 0b00111001; // Active high representation for C
         case 'D':
             return 0b01011110; // Active high representation for D
         case 'E':
             return 0b01111001; // Active high representation for E
         case 'F':
             return 0b01110001; // Active high representation for F
         case 'G':
             return 0b01101111; // Active high representation for G
         case 'H':
             return 0b01110110; // Active high representation for H
         case 'I':
             return 0b00000110; // Active high representation for I
         case 'J':
             return 0b00001110; // Active high representation for J
         case 'K':
             return 0b01110000; // Active high representation for K
         case 'L':
             return 0b00111000; // Active high representation for L
         case 'M':
             return 0b00010101; // Active high representation for M
         case 'N':
             return 0b00110111; // Active high representation for N
         case 'O':
             return 0b00111111; // Active high representation for O
         case 'P':
             return 0b01110011; // Active high representation for P
         case 'Q':
             return 0b01100111; // Active high representation for Q
         case 'R':
             return 0b00110001; // Active high representation for R
         case 'S':
             return 0b01101101; // Active high representation for S
         case 'T':
             return 0b01111000; // Active high representation for T
         case 'U':
             return 0b00111110; // Active high representation for U
         case 'V':
             return 0b00011100; // Active high representation for V
         case 'W':
             return 0b00101010; // Active high representation for W
         case 'X':
             return 0b01000110; // Active high representation for X
         case 'Y':
             return 0b01101110; // Active high representation for Y
         case 'Z':
             return 0b01011011; // Active high representation for Z
         case '-':
             return 0b01000000; // Active high representation for -
         case '_':
             return 0b00001000; // Active high representation for _
         default:
             return 0b00000000; // All segments off
     }
 }

 uint32_t seg2port(uint8_t seg) {
     uint32_t word = 0;
     for (int i = 0; i < 8; i++) {
         if (seg & (1 << i)) {
             word |= 1UL << segment_pins[i];
         }
     }
     return word;
 }

 void num2disp(uint32_t word){

    PortWrite(PC, word, mask);
 }

 void display_select(uint8_t digit){		//De izq a dere 0, 1, 2, 3 <=> DS1, DS2, DS3, DS4 respectivax
     switch(digit){
         case 0:
             gpioWrite(SEL0, HIGH);
             gpioWrite(SEL1, HIGH);
             break;
         case 1:
             gpioWrite(SEL0, LOW);
             gpioWrite(SEL1, HIGH);
             break;
         case 2:
             gpioWrite(SEL0, HIGH);
             gpioWrite(SEL1, LOW);
             break;
         case 3:
             gpioWrite(SEL0, LOW);
             gpioWrite(SEL1, LOW);
             break;
     }
 }

 void led_select(uint8_t led){
	 switch(led){
		 case 0:
			 gpioWrite(PIN_LED_RED, LOW);
			 gpioWrite(PIN_LED_GREEN, HIGH);
			 gpioWrite(PIN_LED_BLUE, HIGH);
			 break;
		 case 1:
			 gpioWrite(PIN_LED_RED, HIGH);
			 gpioWrite(PIN_LED_GREEN, LOW);
			 gpioWrite(PIN_LED_BLUE, HIGH);
			 break;
		 case 2:
			 gpioWrite(PIN_LED_RED, HIGH);
			 gpioWrite(PIN_LED_GREEN, HIGH);
			 gpioWrite(PIN_LED_BLUE, LOW);
			 break;
		 case 3:
			 gpioWrite(PIN_LED_RED, HIGH);
			 gpioWrite(PIN_LED_GREEN, HIGH);
			 gpioWrite(PIN_LED_BLUE, HIGH);
			 break;
	 }
 }
 /*******************************************************************************
  ******************************************************************************/



