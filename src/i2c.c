/*
 * File name: i2c.c
 * File description: This file configures the I2C peripheral and implements I2C communication with SI7021 sensor
 * Date: 21-Sep-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides weeks 3-4
 *    [2] SI7201 A20 Datasheet https://www.silabs.com/documents/public/data-sheets/Si7021-A20.pdf
 *    [3] Silicon Labs Developer Documentation https://docs.silabs.com/gecko-platform/4.3/platform-emlib-efr32xg1/
 */


// Include logging for this file
#define INCLUDE_LOG_DEBUG 1 //comment this out while taking energy measurements
#include "src/log.h"
#include "src/i2c.h"


uint8_t cmd_data;
uint8_t read_data[2];
I2C_TransferSeq_TypeDef transferSequence;

// Initialize the I2C hardware with required configuration
I2CSPM_Init_TypeDef I2C_Config =
    {
        .port = I2C0,
        .sclPort = gpioPortC,
        .sclPin = 10,
        .sdaPort = gpioPortC,
        .sdaPin = 11,
        .portLocationScl = 14,
        .portLocationSda = 16,
        .i2cRefFreq = 0,
        .i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
        .i2cClhr = i2cClockHLRStandard
    };

/*
 * @brief This function initiates the I2C peripheral
 * @param none
 * @return none
 */
void i2cInit(void)
{
  I2CSPM_Init(&I2C_Config);
}

/*
 * @brief Write a command to the SI7021 sensor over I2C.
 * @param command , the command to be written to the transmit buffer
 * @return none
 */
void Write_I2C(uint8_t command)
{
  I2C_TransferReturn_TypeDef transferStatus;
  i2cInit();
  cmd_data = command;
  transferSequence.addr = SI7021_DEVICE_ADDR << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_WRITE;
  transferSequence.buf[0].data = &cmd_data; // pointer to data to write
  transferSequence.buf[0].len = sizeof(cmd_data);
  NVIC_EnableIRQ(I2C0_IRQn);
  transferStatus = I2C_TransferInit (I2C0, &transferSequence);
  if (transferStatus < 0)
    {
      LOG_ERROR("I2CSPM_Transfer: I2C bus write of cmd=%d failed\r\n", cmd_data);
    }
}

/*
 *  @brief Read data from the SI7021 sensor over I2C.
 *  @param none
 *  @return none
 */
void Read_I2C(void)
{
  I2C_TransferReturn_TypeDef transferStatus;
  i2cInit();
  transferSequence.addr = SI7021_DEVICE_ADDR << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_READ;
  transferSequence.buf[0].data = read_data; // pointer to data to write
  transferSequence.buf[0].len = sizeof(read_data);
  NVIC_EnableIRQ(I2C0_IRQn);

  transferStatus = I2C_TransferInit (I2C0, &transferSequence);
  if (transferStatus < 0)
    {
      LOG_ERROR("I2CSPM_Transfer: I2C bus read failed\r\n");
    }
}
/*
 *  @brief Read temperature from the SI7021 sensor , convert it to C and log it.
 *  @param none
 *  @return none
 */
int32_t read_temp_from_si7021(void)
{
  //Swapping lower 8 bits with higher 8 bit
  uint16_t swapped_read_data = ((read_data[0])<<8) | (read_data[1]);
  //Converting to celsius
  int32_t temp = (((swapped_read_data)*175.72)/65536) - 46.85; //modified datatype to int32_t from float to accomodate A5 changes
  //LOG_INFO("Read temperature = %d Celsius\r\n",temp);
  return temp;
}


