/*****************************************************************************
 * @file 
 * @brief 
 *******************************************************************************
 Copyright 2022 GL-iNet. https://www.gl-inet.com/

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
#ifndef GL_COMMON_H
#define GL_COMMON_H
#include <stdint.h>
#include "gl_type.h"
#include "sl_status.h"

/***********************************************************************************************//**
 *  \brief  The MAC address of uint8_t type convets to string.
 *  \param[in]    adr The MAC address of uint8_t type.
 *  \param[out]   str Buffer used for storing the mac address. 
 *  \return 0 means success, None-zero means failed.
 **************************************************************************************************/
int addr2str(BLE_MAC adr, char* str);

/***********************************************************************************************//**
 *  \brief  The MAC address of string type convets to uint8_t type.
 *  \param[in]    adr The MAC address of string type.
 *  \param[out]   address Buffer used for storing the mac address. 
 *  \return 0 means success, None-zero means failed.
 **************************************************************************************************/
int str2addr(char* str, BLE_MAC address);

/***********************************************************************************************//**
 *  \brief  The string convets to uint8_t array.
 *  \param[in]    src The string will be converted.
 *  \param[out]   dst The uint8_t array used for storing the result.
 *  \return 0 means success, None-zero means failed.
 **************************************************************************************************/
int str2array(uint8_t* dst, char* src, int len);

/***********************************************************************************************//**
 *  \brief  Hexadecimal conversion to a string.
 *  \param[in]   head  The hexadecimal will be converted.
 *  \param[in]   len   The length of the hexadecimal.
 *  \param[in]   value Buffer used for storing the coverted result.
 *  \return 0 means success, None-zero means failed.
 **************************************************************************************************/
int hex2str(uint8_t* head, int len, char* value);

/***********************************************************************************************//**
 *  \brief  Reverse endian conversion
 *  \param[in]   header  The value will be reversed.
 *  \param[in]   length  The length of the value.
 **************************************************************************************************/
void reverse_endian(uint8_t* header, uint8_t length);

sl_status_t ble_write_long_data(uint16_t len, uint8_t *adv_data);

#endif