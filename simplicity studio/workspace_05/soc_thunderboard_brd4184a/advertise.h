/***************************************************************************//**
 * @file
 * @brief Thunderboard advertising header
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef ADVERTISE_H
#define ADVERTISE_H

#include <stdint.h>

void advertise_init(uint32_t unique_id);
void advertise_start(void);
void advertise_stop(void);

#endif // ADVERTISE_H
