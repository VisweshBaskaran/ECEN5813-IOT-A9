/*
  gpio.c

   Created on: Dec 12, 2018
       Author: Dan Walkes
   Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

   March 17
   Dave Sluiter: Use this file to define functions that set up or control GPIOs.

   Jan 24, 2023
   Dave Sluiter: Cleaned up gpioInit() to make it less confusing for students regarding
                 drive strength setting. 

 *
 * Student edit: Add your name and email address here:
 * @student   Visweshwaran Baskaran, viswesh.baskaran@colorado.edu
 *
 */


// *****************************************************************************
// Students:
// We will be creating additional functions that configure and manipulate GPIOs.
// For any new GPIO function you create, place that function in this file.
// *****************************************************************************


#include "gpio.h"
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

// Student Edit: Define these, 0's are placeholder values.
//
// See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
// and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
// to determine the correct values for these.
// If these links have gone bad, consult the reference manual and/or the datasheet for the MCU.
// Change to correct port and pins:





// Set GPIO drive strengths and modes of operation
void gpioInit()
{
  // Student Edit:

  // Set the port's drive strength. In this MCU implementation, all GPIO cells
  // in a "Port" share the same drive strength setting.
  GPIO_DriveStrengthSet(LED_port, gpioDriveStrengthWeakAlternateWeak); // Weak, 1mA

  // Set the GPIO mode of operation
  GPIO_PinModeSet(LED_port, LED0_pin, gpioModePushPull, false);

  //DOS GPIO_DriveStrengthSet(LED_port, gpioDriveStrengthWeakAlternateWeak); // Weak, 1mA
  // Set the GPIO mode of operation
  GPIO_PinModeSet(LED_port, LED1_pin, gpioModePushPull, false);


  GPIO_DriveStrengthSet(SI7021_port, gpioDriveStrengthWeakAlternateWeak);
  // Set the sensor mode of operation
  GPIO_PinModeSet(SI7021_port, SI7021_pin, gpioModePushPull, false);


  // DOS GPIO interrupts are enabled for the duration of execution, but first disable and clear
  // any spurious IRQs that may be hanging around.
  GPIO_IntDisable (0xFFFFFFFF);  // disable GPIO IRQs
  GPIO_IntClear   (0xFFFFFFFF);  // clear any previous, spurious IRQs

  //DOS GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInputPullFilter, true);
  GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInput, true); // DOS
  GPIO_ExtIntConfig (PB0_port, PB0_pin, PB0_pin, true, true, true);

  //DOS GPIO_PinModeSet(PB1_port, PB1_pin, gpioModeInputPullFilter, true);
  GPIO_PinModeSet(PB1_port, PB1_pin, gpioModeInput, true);
  GPIO_ExtIntConfig (PB1_port, PB1_pin, PB1_pin, true, true, true); // DOS



} // gpioInit()


void gpioLed0SetOn()
{
  GPIO_PinOutSet(LED_port, LED0_pin);
}


void gpioLed0SetOff()
{
  GPIO_PinOutClear(LED_port, LED0_pin);
}


void gpioLed1SetOn()
{
  GPIO_PinOutSet(LED_port, LED1_pin);
}


void gpioLed1SetOff()
{
  GPIO_PinOutClear(LED_port, LED1_pin);
}

void si7021SetOn()
{
  GPIO_PinOutSet(SI7021_port, SI7021_pin);
}

void si7021SetOff()
{
  GPIO_PinOutClear(SI7021_port, SI7021_pin);
}

void LCDSetOn()
{
  GPIO_PinOutSet(LCD_port, LCD_pin);
}
void LCDSetoff()
{
  GPIO_PinOutClear(LCD_port, LCD_pin);
}
void gpioSetDisplayExtcomin(bool last_extcomin_state_high)
{
  if(last_extcomin_state_high)
    LCDSetOn();
  else if(!last_extcomin_state_high)
    LCDSetoff();
}

