/*
   gpio.h
  
    Created on: Dec 12, 2018
        Author: Dan Walkes

    Updated by Dave Sluiter Sept 7, 2020. moved #defines from .c to .h file.
    Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

    Editor: Feb 26, 2022, Dave Sluiter
    Change: Added comment about use of .h files.

 *
 * Student edit: Add your name and email address here:
 * @student    Visweshwaran Baskaran, viswesh.baskaran@colorado.edu
 *
 * @reference 1) https://www.silabs.com/documents/public/application-notes/an0012-efm32-gpio.pdf
 *            2) https://www.silabs.com/documents/public/user-guides/ug279-brd4104a-user-guide.pdf
 *            2) https://www.silabs.com/documents/public/user-guides/ug281-brd4159a-user-guide.pdf
 
 */


// Students: Remember, a header file (a .h file) generally defines an interface
//           for functions defined within an implementation file (a .c file).
//           The .h file defines what a caller (a user) of a .c file requires.
//           At a minimum, the .h file should define the publicly callable
//           functions, i.e. define the function prototypes. #define and type
//           definitions can be added if the caller requires theses.


#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_

//Macros
// Reference [1]
#define LED_port   (gpioPortF)
#define LCD_port (gpioPortD)
#define SI7021_port (gpioPortD)
#define LED0_pin   (4)
#define LED1_pin   (5)
#define SI7021_pin (15)
#define LCD_pin   (13) //PD13 DISP_EXTCOMIN [2]
#define PB0_port (gpioPortF)
#define PB0_pin 6 //[3]
//Header files
#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>

// Function prototypes
void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void LCDSetOn();
void LCDSetoff();
void gpioLed1SetOff();
void si7021SetOn();
void si7021SetOff();
void gpioSetDisplayExtcomin(bool last_extcomin_state_high);


#endif /* SRC_GPIO_H_ */
