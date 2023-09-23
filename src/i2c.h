/*
 * File name: i2c.h
 * File description: This file declares the APIs that configures the I2C peripheral and implements I2C communication with SI7021 sensor
 * Date: 21-Sep-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides week 3
 *    [2] SI7201 A20 Datasheet https://www.silabs.com/documents/public/data-sheets/Si7021-A20.pdf
 *    [3] Silicon Labs Developer Documentation https://docs.silabs.com/gecko-platform/4.3/platform-emlib-efr32xg1/
 */


#ifndef SRC_I2C_H_
#define SRC_I2C_H_
#include "app.h"
#include "em_gpio.h"
#include "sl_i2cspm.h"
#include "em_i2c.h"
#include "src/timers.h"

#define SI7021_DEVICE_ADDR 0x40

/*
 * @brief This function initiates the I2C peripheral
 *
 * @param none
 *
 * @return none
 */
void i2cInit(void);

/*
 * @brief Write a command to the SI7021 sensor over I2C.
 *
 * @param command , the command to be written to the transmit buffer
 *
 * @return none
 */
void Write_I2C(uint8_t command);
/*
 *  @brief Read data from the SI7021 sensor over I2C.
 *
 *  @param none
 *
 *  @return none
 */
void Read_I2C(void);

/*
 *  @brief Read temperature from the SI7021 sensor , convert it to C and log it.
 *
 *  @param none
 *
 *  @return none
 */
void read_temp_from_si7021(void);
#endif /* SRC_I2C_H_ */
