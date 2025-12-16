/***************************************************************************//**
  @file     gpio.h
  @brief    Simple GPIO Pin services, similar to Arduino
  @author   Nicol s Magliola
 ******************************************************************************/

 #ifndef _GPIO_H_
 #define _GPIO_H_
 
 /*******************************************************************************
  * INCLUDE HEADER FILES
  ******************************************************************************/
 
 #include <stdint.h>
 #include <stdbool.h>
 
 
 /*******************************************************************************
  * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
  ******************************************************************************/
 
 // Ports
 enum { PA, PB, PC, PD, PE };
 
 // Convert port and number into pin ID
 // Ex: PTB5  -> PORTNUM2PIN(PB,5)  -> 0x25
 //     PTC22 -> PORTNUM2PIN(PC,22) -> 0x56
 #define PORTNUM2PIN(p,n)    (((p)<<5) + (n))
 #define PIN2PORT(p)         (((p)>>5) & 0x07) 
 #define PIN2NUM(p)          ((p) & 0x1F)
 
 
 // Modes
 #ifndef INPUT
 #define INPUT               0
 #define OUTPUT              1
 #define INPUT_PULLUP        2
 #define INPUT_PULLDOWN      3
 #endif // INPUT
 
 
 // Digital values
 #ifndef LOW
 #define LOW     0
 #define HIGH    1
 #endif // LOW
  
#define NUM_PORTS 5  // Cantidad de puertos (A, B, C, D, E)

 // IRQ modes
 enum {
      GPIO_IRQ_DISABLE,
      GPIO_IRQ_DMA_RISING_EDGE,
      GPIO_IRQ_DMA_FALLING_EDGE,
      GPIO_IRQ_DMA_EITHER_EDGE,
      GPIO_IRQ_LOGIC_LOW,
      GPIO_IRQ_RISING_EDGE,
      GPIO_IRQ_FALLING_EDGE,
      GPIO_IRQ_EITHER_EDGE,
      GPIO_IRQ_LOGIC_HIGH,
      GPIO_IRQ_CANT_MODES
 };
 
 typedef enum
 {
 	PORT_mAnalog,
 	PORT_mGPIO,
 	PORT_mAlt2,
 	PORT_mAlt3,
 	PORT_mAlt4,
 	PORT_mAlt5,
 	PORT_mAlt6,
 	PORT_mAlt7,
 } PORTMux_t;
 
 /*******************************************************************************
  * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
  ******************************************************************************/
 
 typedef uint8_t pin_t;
 
 typedef void (*pinIrqFun_t)(void);
 
 
 /*******************************************************************************
  * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
  ******************************************************************************/
 
 /*******************************************************************************
  * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
  ******************************************************************************/
 
 /**
  * @brief Configures the specified pin to behave either as an input or an output
  * @param pin the pin whose mode you wish to set (according PORTNUM2PIN)
  * @param mode INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.
  */
 void gpioMode (pin_t pin, uint8_t mode);
 
 /**
  * @brief Configures how the pin reacts when an IRQ event ocurrs
  * @param pin the pin whose IRQ mode you wish to set (according PORTNUM2PIN)
  * @param irqMode disable, risingEdge, fallingEdge or bothEdges
  * @param irqFun function to call on pin event
  * @return Registration succeed
  */
 bool gpioIRQ (pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun);
 
 /**
  * @brief Write a HIGH or a LOW value to a digital pin
  * @param pin the pin to write (according PORTNUM2PIN)
  * @param val Desired value (HIGH or LOW)
  */
 void gpioWrite (pin_t pin, bool value);
 
 /**
  * @brief Toggle the value of a digital pin (HIGH<->LOW)
  * @param pin the pin to toggle (according PORTNUM2PIN)
  */
 void gpioToggle (pin_t pin);
 
 /**
  * @brief Reads the value from a specified digital pin, either HIGH or LOW.
  * @param pin the pin to read (according PORTNUM2PIN)
  * @return HIGH or LOW
  */
 bool gpioRead (pin_t pin);
 
 /**
  * @brief Write a value to a port (all pins at once)
  * 
  * @param port Port number (0-4)
  * @param value Value to write to the port (32 bits)
  * @param mask Mask to apply to the value (32 bits)
  * @note  Only the bits in the mask will be written to the port
  */
 void PortWrite(uint8_t port, uint32_t value, uint32_t mask);
 
 /*******************************************************************************
  ******************************************************************************/
 
 #endif // _GPIO_H_
 
