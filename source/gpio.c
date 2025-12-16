/***************************************************************************//**
  @file     gpio.c
  @brief    Source file for gpio driver.
  @author   Felipe Mendia
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
 #include "gpio.h"
 #include "MK64F12.h"
 #include "core_cm4.h"
 #include "hardware.h"
 #include <stddef.h>
 
 /*******************************************************************************
  * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
  ******************************************************************************/

 #define NUM_PINS  32 // Cantidad de pines por puerto
 #define ISF_DISABLE 0b0000
 #define ISF_DMA_RISING_EDGE 0b0001
 #define ISF_DMA_FALLING_EDGE 0b0010
 #define ISF_DMA_EITHER_EDGE 0b0011
 #define ISF_LOGIC_LOW 0b1000
 #define ISF_RISING_EDGE 0b1001
 #define ISF_FALLING_EDGE 0b1010
 #define ISF_EITHER_EDGE 0b1011
 #define ISF_LOGIC_HIGH 0b1100
 
 /*******************************************************************************
  * GLOBAL PRIVATE VARIABLES
  ******************************************************************************/
 
  static pinIrqFun_t gpioCallbacks[NUM_PORTS][NUM_PINS] = {NULL}; // Array de callbacks
  static const uint32_t IRQC_MODES[] = {ISF_DISABLE, ISF_DMA_RISING_EDGE, ISF_DMA_FALLING_EDGE, ISF_DMA_EITHER_EDGE,
                                        ISF_LOGIC_LOW, ISF_RISING_EDGE, ISF_FALLING_EDGE, ISF_EITHER_EDGE,
                                        ISF_LOGIC_HIGH};
 
  static PORT_Type* const PORT_PTRS[NUM_PORTS] = PORT_BASE_PTRS;
  static GPIO_Type* const GPIO_PTRS[NUM_PORTS] = GPIO_BASE_PTRS;
 
 /*******************************************************************************
  * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
  ******************************************************************************/
 
  static void IRQHandler (uint32_t portNum);
 /*******************************************************************************
  *******************************************************************************
                         GLOBAL FUNCTION DEFINITIONS
  *******************************************************************************
  ******************************************************************************/
 void gpioMode (pin_t pin, uint8_t mode)
 {
 
     PORT_Type * port = PORT_PTRS[PIN2PORT(pin)];
     GPIO_Type * gpio = GPIO_PTRS[PIN2PORT(pin)];
 
     //Enable port clk
     if (port == PORTA){
             SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
         }
         else if (port == PORTB){
             SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
         }
         else if (port == PORTC){
             SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
             }
         else if (port == PORTD){
             SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
             }
         else if (port == PORTE){
             SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
             }
 
     // configure mode gpio (mux)
     port->PCR[PIN2NUM(pin)] = (port->PCR[PIN2NUM(pin)] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
 
     //configure input/output
     if (mode == OUTPUT)
     {
         gpio->PDDR |= (1 << PIN2NUM(pin));
     }
     else if (mode == INPUT)
     {
         gpio->PDDR &= ~(1 << PIN2NUM(pin));
 
         if (mode == INPUT_PULLUP)
         {
             port->PCR[PIN2NUM(pin)] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
         }
         else if (mode == INPUT_PULLDOWN)
         {
             port->PCR[PIN2NUM(pin)] |= PORT_PCR_PE_MASK;
         }
     }
 
 }
 
 void gpioWrite (pin_t pin, bool value)
 {
     //PORT_Type * port = PORT_PTRS[PIN2PORT(pin)];
     GPIO_Type * gpio = GPIO_PTRS[PIN2PORT(pin)];
     //getPort(pin, &port, &gpio); //
 
     gpio->PDOR = (gpio->PDOR & ~(1 << PIN2NUM(pin))) | (value << PIN2NUM(pin));
 
 }
 
 void gpioToggle (pin_t pin)
 {
     //PORT_Type * port = PORT_PTRS[PIN2PORT(pin)];
     GPIO_Type * gpio = GPIO_PTRS[PIN2PORT(pin)];
     //getPort(pin, &port, &gpio);
 
     gpio->PTOR |= (1<<PIN2NUM(pin));
 
 }
 
 bool gpioRead (pin_t pin)
 {
     //PORT_Type * port = PORT_PTRS[PIN2PORT(pin)];
     GPIO_Type * gpio = GPIO_PTRS[PIN2PORT(pin)];
     //getPort(pin, &port, &gpio);
 
     return (gpio->PDIR & (1<<PIN2NUM(pin)));
 }
 
 bool gpioIRQ (pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun)
 {
     if(irqFun == NULL) {return false; }
     if ( irqMode >= GPIO_IRQ_CANT_MODES ) return false;
 
     PORT_Type * port = PORT_PTRS[PIN2PORT(pin)];
     //GPIO_Type * gpio = GPIO_PTRS[PIN2PORT(pin)];
 
     uint8_t pinNumber = PIN2NUM(pin);
     uint8_t portIndex = PIN2PORT(pin);
 
     gpioCallbacks[portIndex][pinNumber] = irqFun; //Guardar la funcion callback
 
     //modo de interrupcion
     port->PCR[PIN2NUM(pin)] &= ~PORT_PCR_IRQC_MASK;
     port->PCR[PIN2NUM(pin)] =  port->PCR[PIN2NUM(pin)] | PORT_PCR_IRQC(IRQC_MODES[irqMode]) | PORT_PCR_ISF_MASK;
 
     //enable iqr in port and set in desired mode
     if(port == PORTA)
     {
         NVIC_EnableIRQ(PORTA_IRQn);
     }
     else if(port == PORTB)
     {
         NVIC_EnableIRQ(PORTB_IRQn);
     }
     else if(port == PORTC)
     {
         NVIC_EnableIRQ(PORTC_IRQn);
     }
     else if(port == PORTD)
     {
         NVIC_EnableIRQ(PORTD_IRQn);
     }
     else if(port == PORTE)
     {
         NVIC_EnableIRQ(PORTE_IRQn);
     }
     else
     {
         return false;
     }
     return true;
 }
 
 /*******************************************************************************
  *******************************************************************************
                         LOCAL FUNCTION DEFINITIONS
  *******************************************************************************
  ******************************************************************************/
 
 static void IRQHandler( uint32_t portNum)
 {
     PORT_Type * port = PORT_PTRS[portNum];
     for (int pin = 0; pin < NUM_PINS; pin++)
     {
         if ((port->ISFR) >> pin & 0b1)
         {
             port->PCR[pin] |= PORT_PCR_ISF_MASK;
              if (gpioCallbacks[portNum][pin] != NULL)
              {
                  gpioCallbacks[portNum][pin]();  // Ejecutar la función callback asociada
              }
         }
     }
 }
 
 
 __ISR__ PORTA_IRQHandler(void) { IRQHandler(PA); }
 __ISR__ PORTB_IRQHandler(void) { IRQHandler(PB); }
 __ISR__ PORTC_IRQHandler(void) { IRQHandler(PC); }
 __ISR__ PORTD_IRQHandler(void) { IRQHandler(PD); }
 __ISR__ PORTE_IRQHandler(void) { IRQHandler(PE); }
 
 
 
void PortWrite(uint8_t port, uint32_t value, uint32_t mask){
    //Assume the output pins have already been configured as output and there is no use for input pins

    GPIO_Type *gpioPtr = ((GPIO_Type *[]) GPIO_BASE_PTRS)[port]; //Pointer to the correct GPIO
    gpioPtr->PSOR = (value & mask); //Set the bits in the PDOR register
    gpioPtr->PCOR = (~value & mask); //Clear the bits in the PDOR register
}
