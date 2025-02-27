/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV, LiQuiBit VOF.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* \file
 *
 *
 * @author contact@liquibit.be
 */

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include "platform_defs.h"
#include "stm32_common_eeprom.h"
#include "stm32_common_mcu.h"
#include "stm32_device.h"

#include "blockdevice_ram.h"
#include "fs.h"
#include "hwblockdevice.h"
#include "hwi2c.h"

#ifndef PLATFORM_LIQUIBUS_V2
#error Mismatch between the configured platform and the actual platform. Expected LIQUIBUS to be defined
#endif

/********************
 * LED DEFINITIONS *
 *******************/

#define LED1 PIN(1, 15)
#define LED2 PIN(1, 14)
#define LED_WHITE 0

/**************************
 * USERBUTTON DEFINITIONS *
 *************************/

#define BUTTON1_PIN PIN(1, 12)
// #define BUTTON2_PIN PIN(0, 5)

#if defined(USE_SX127X) || defined(USE_NETDEV_DRIVER)

#define SX127x_SPI_INDEX 0
#define SX127x_SPI_PIN_CS PIN(0, 15)
#define SX127x_SPI_BAUDRATE 8000000
#define SX127x_DIO0_PIN PIN(1, 4)
#define SX127x_DIO1_PIN PIN(1, 1)
#ifdef PLATFORM_SX127X_USE_DIO3_PIN
#define SX127x_DIO3_PIN PIN(2, 13)
#endif
#ifdef PLATFORM_SX127X_USE_RESET_PIN
#define SX127x_RESET_PIN PIN(2, 0)
#endif
#ifdef PLATFORM_SX127X_USE_VCC_TXCO
#define SX127x_VCC_TXCO PIN(7, 1)
#endif
#endif

// TODO temp disabled, until set_antenna_switch() is ported
#define PLATFORM_USE_ABZ // this platform is based on the Murata ABZ module
// Antenna switching uses 3 pins on murata ABZ module
#define ABZ_ANT_SW_RX_PIN PIN(0, 1)
#define ABZ_ANT_SW_TX_PIN PIN(2, 2)
#define ABZ_ANT_SW_PA_BOOST_PIN PIN(2, 1)

/** Platform BD drivers*/
extern blockdevice_t* const metadata_blockdevice;
extern blockdevice_t* const persistent_files_blockdevice;
extern blockdevice_t* const volatile_blockdevice;
#define PLATFORM_METADATA_BLOCKDEVICE metadata_blockdevice
#define PLATFORM_PERMANENT_BLOCKDEVICE persistent_files_blockdevice
#define PLATFORM_VOLATILE_BLOCKDEVICE volatile_blockdevice

i2c_handle_t* platf_get_i2c_handle();

#endif
