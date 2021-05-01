/**
 * @file    pb.h
 * @brief   Pushbutton driver header file.
 */

/*******************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *
 * $Date: 2018-08-09 18:45:02 -0500 (Thu, 09 Aug 2018) $
 * $Revision: 36818 $
 *
 ******************************************************************************/


#ifndef _PB_H_
#define _PB_H_

#include "gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/***** Global Variables *****/
extern const gpio_cfg_t pb_pin[];
extern const unsigned int num_pbs;

/***** Function Prototypes *****/

/**
 * @brief   Initialize all push buttons.
 * @returns #E_NO_ERROR if everything is successful, error if unsuccessful.
 */
int PB_Init(void);

/**
 * @brief   Type alias \c pb_callback for function of type: \code void pb_callback(void * pb) \endcode
 * 			To recieve notification of a push button event, define a callback function and pass it as a
 * 			pointer to the \c PB_RegisterCallback function.
 * @param   pb        Pointer to the push button index that resulted in the callback.
 */
typedef void (*pb_callback)(void *pb);

/**
 * @brief   Register a callback. Configure and enable an interrupt. Calling this function
 *          with a NULL pointer will disable the interrupt and unregister the callback.
 * @param   pb          push button index for registering the callback against
 * @param   callback    Callback function pointer of type \c pb_callback
 * @returns #E_NO_ERROR if everything is successful, error if unsuccessful.
 */
int PB_RegisterCallback(unsigned int pb, pb_callback callback);

/**
 * @brief   Enable a callback interrupt. PB_RegisterCallback should be called prior.
 * @param   pb          push button index
 */
void PB_IntEnable(unsigned int pb);

/**
 * @brief   Disable a callback interrupt.
 * @param   pb          push button index
 */
void PB_IntDisable(unsigned int pb);

/**
 * @brief   Clear a callback interrupt.
 * @param   pb          push button index
 */
void PB_IntClear(unsigned int pb);

/**
 * @brief   Get the current state of the pushbutton.
 * @param   pb          push button index
 * @returns TRUE if the the button is pressed, FALSE otherwise.
 */
int PB_Get(unsigned int pb);

#ifdef __cplusplus
}
#endif

#endif /* _PB_H_ */
