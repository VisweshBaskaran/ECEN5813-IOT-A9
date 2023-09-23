/*
 * File name: oscillator.c
 * File description: This file declares the API that initializes oscillator peripherals (LFXO and ULFRCO)
 * Date: 14-Sep-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides week 2-3
 */

#ifndef SRC_OSCILLATORS_H_
#define SRC_OSCILLATORS_H_

//Required header files
#include "app.h"

//Macros


#define LFXO (32768)
#define PRESCALER_LFXO (4)

#define ULFRCO (1000)


/*
 * @brief This function configures the oscillator and clock settings according to the energy mode set.
 *
 * @param none
 *
 * @return none
 */
void oscInit(void);
#endif /* SRC_OSCILLATORS_H_ */
