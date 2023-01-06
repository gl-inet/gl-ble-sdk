/*****************************************************************************
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
#ifndef _GL_ERRNO_H_
#define _GL_ERRNO_H_

#include <stdint.h>
/**
 * @defgroup RETURN_CODE RETURN_CODE
 * The return value definition of the API
 */

/**
 * @defgroup GL_RETURN_CODE GL_RETURN_CODE
 * @ingroup RETURN_CODE
 * RETURN CODE define by GL-iNet
 * @{
 */

/*
 * 0x0000 - 0x00FF    Error code defined by GL-iNet
 * 0x0100 - 0xFFFF    Error code defined by ble module manufacturer
 * Error code defined by Silabs: \src\daemon\bledriver\silabs\bg_errorcodes.h
 * 
*/

enum gl_error_spaces {
/**
 * @brief The base num of return code for GL-iNet.
 */
	GL_ERR_BASE					= 0,

/**
 * @brief The base num of return code for chip manufacturer.
 */
 	MANUFACTURER_ERR_BASE 		= 0x0100,
};

typedef int32_t GL_RET;

typedef enum gl_error {

/**
 * @brief The generic "no error" message.
 */
	GL_SUCCESS      	   			= GL_ERR_BASE + 0,            

/**
 * @brief The generic "unknow error" message.
 */
	GL_UNKNOW_ERR					= GL_ERR_BASE + 1, 			

/**
 * @brief Response missing.
 */
	GL_ERR_RESP_MISSING				= GL_ERR_BASE + 2, 			

/**
 * @brief Event missing.
 */
	GL_ERR_EVENT_MISSING			= GL_ERR_BASE + 3, 			

/**
 * @brief Param missing.
 */	
	GL_ERR_PARAM_MISSING			= GL_ERR_BASE + 4, 			

/**
 * @brief Message error.
 */	
	GL_ERR_MSG						= GL_ERR_BASE + 5, 			

/**
 * @brief Param error.
 */	
	GL_ERR_PARAM					= GL_ERR_BASE + 6, 			

/**
 * @brief Invoking function in error mode.
 */	
	GL_ERR_INVOKE					= GL_ERR_BASE + 7, 

/**
 * @brief Unsupprot hardware model.
 */	
	GL_ERR_UNSUPPORT_MODEL			= GL_ERR_BASE + 8, 
}errcode_t;

#endif