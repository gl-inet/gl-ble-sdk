/*****************************************************************************
 * @file 
 * @brief 
 *******************************************************************************
 Copyright 2020 GL-iNet. https://www.gl-inet.com/

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ******************************************************************************/
#ifndef GL_UART_H
#define GL_UART_H

#include <stdint.h>
/***********************************************************************************************//**
 *  \brief  Open the serial port.
 *  \param[in]  port Serial port to use.
 *  \param[in]  baudRate Baud rate to use.
 *  \param[in]  rtsCts Enable/disable hardware flow control.
 *  \param[in]  timeout Constant used to calculate the total time-out period for read operations, in
 *              milliseconds.
 *  \return  0 on success, -1 on failure.
 **************************************************************************************************/
int32_t uartOpen(int8_t* port, uint32_t baudRate, uint32_t rtsCts, int32_t timeout);

/***********************************************************************************************//**
 *  \brief  Close the serial port.
 *  \return  0 on success, -1 on failure.
 **************************************************************************************************/
int32_t uartClose(void);

/***********************************************************************************************//**
 *  \brief  Blocking read data from serial port. The function will block until the desired amount
 *          has been read or an error occurs.
 *  \note  In order to use this function the serial port has to be configured blocking. This can be
 *         done by calling uartOpen() with 'timeout = 0'.
 *  \param[in]  dataLength The amount of bytes to read.
 *  \param[out]  data Buffer used for storing the data.
 *  \return  The amount of bytes read or -1 on failure.
 **************************************************************************************************/
int32_t uartRx(uint32_t dataLength, uint8_t* data);

/***********************************************************************************************//**
 *  \brief  Non-blocking read from serial port.
 *  \note  A truly non-blocking operation is possible only if uartOpen() is called with timeout
 *         parameter set to 0.
 *  \param[in]  dataLength The amount of bytes to read.
 *  \param[out]  data Buffer used for storing the data.
 *  \return  The amount of bytes read, 0 if configured serial blocking time interval elapses or -1
 *           on failure.
 **************************************************************************************************/
int32_t uartRxNonBlocking(uint32_t dataLength, uint8_t* data);

/***********************************************************************************************//**
 *  \brief  Return the number of bytes in the input buffer.
 *  \return  The number of bytes in the input buffer or -1 on failure.
 **************************************************************************************************/
int32_t uartRxPeek(void);

/***********************************************************************************************//**
 *  \brief  Write data to serial port. The function will block until
 *          the desired amount has been written or an error occurs.
 *  \param[in]  dataLength The amount of bytes to write.
 *  \param[in]  data Buffer used for storing the data.
 *  \return  The amount of bytes written or -1 on failure.
 **************************************************************************************************/
int32_t uartTx(uint32_t dataLength, uint8_t* data);


int32_t uartCacheClean(void);

#endif