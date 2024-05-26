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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AcuRev_1312_RCT.h"
#include "hwuart.h"
#include "mmodbus.h"
#include "math.h"

#define MODBUS_MAX_RETRIES 10


#ifdef true
#include "log.h"
#define DPRINT(...) log_print_string(__VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif

static uart_handle_t* uart;

#define password 0

#define device_address 1
#define modbus_timeout 1000
#define Total_Real_Energy_Imported_register 4211 // size 32 bit
#define	Real_Energy_Sunpec_Scale_Factor_register 4219 // -3 - 0
#define Total_Apparent_Energy_Imported_register 4228 // size 32 bit
#define	Apparent_Energy_Sunspec_Scale_Factor_register 4236 // -3 - 0

#define Total_Real_Energy_Phase_A_register 4213 // size 32 bit
#define Total_Real_Energy_Phase_B_register 4215 // size 32 bit
#define Total_Real_Energy_Phase_C_register 4217 // size 32 bit
#define Real_Energy_Scale_Factor_register 4219 // -3 - 0

#define Total_Apparent_Energy_Phase_A_register 4230 // size 32 bit
#define Total_Apparent_Energy_Phase_B_register 4232 // size 32 bit
#define Total_Apparent_Energy_Phase_C_register 4234 // size 32 bit
#define Apparent_Energy_Scale_Factor_register 4236 // -3 - 0

#define Voltage_Phase_A_register 4173 // size 16 bit
#define Voltage_Phase_B_register 4174 // size 16 bit
#define Voltage_Phase_C_register 4175 // size 16 bit
#define Voltage_Scale_Factor_register 4180 // -2 - 2

#define Current_Phase_A_register 4168 // size 16 bit
#define Current_Phase_B_register 4169 // size 16 bit
#define Current_Phase_C_register 4170 // size 16 bit
#define Current_Scale_Factor_register 4171 // -3 - 5


void acurev_1312_rct_init()
{
    uart = uart_init(0, 19200, 0);
    uart_enable(uart);
    uart_set_rx_interrupt_callback(uart, &modbus_callback_stack);
    uart_rx_interrupt_enable(uart);
    
    mmodbus_init(modbus_timeout);
    mmodbus_set32bitOrder(MModBus_32bitOrder_CDAB);
    DPRINT("acurev inited");
}

bool acurev_get_real_energy(int64_t *real_energy_a, int64_t *real_energy_b, int64_t *real_energy_c)
{
    uint32_t data_a, data_b, data_c;
    uint16_t raw_scale;
    int16_t scale;
    bool success = false;
    uint8_t retry_counter = 0;
    while (!success && retry_counter <= MODBUS_MAX_RETRIES)
    {
        success = mmodbus_readHoldingRegister32i(device_address, Total_Real_Energy_Phase_A_register, &data_a);
        success &= mmodbus_readHoldingRegister32i(device_address, Total_Real_Energy_Phase_B_register, &data_b);
        success &= mmodbus_readHoldingRegister32i(device_address, Total_Real_Energy_Phase_C_register, &data_c);
        success &= mmodbus_readHoldingRegister16i(device_address, Real_Energy_Scale_Factor_register, &raw_scale);
        // Transform raw scale to signed integer
        scale = (int16_t)raw_scale;
        *real_energy_a = (int64_t)((int32_t)data_a * pow(10, (int16_t)scale + 3));
        *real_energy_b = (int64_t)((int32_t)data_b * pow(10, (int16_t)scale + 3));
        *real_energy_c = (int64_t)((int32_t)data_c * pow(10, (int16_t)scale + 3));
        retry_counter++;
        DPRINT("Attempt %d: Raw Data A: %d, Raw Data B: %d, Raw Data C: %d, Raw Scale: %d, Scale: %d, real_energy A: %d, real_energy B: %d, real_energy C: %d", 
        retry_counter + 1, data_a, data_b, data_c, raw_scale, scale, *real_energy_a, *real_energy_b, *real_energy_c); 
    }
    return success;
}

bool acurev_get_apparent_energy(int64_t *apparent_energy_a, int64_t *apparent_energy_b, int64_t *apparent_energy_c)
{
    uint32_t data_a, data_b, data_c;
    uint16_t raw_scale;
    int16_t scale;
    bool success = false;
    uint8_t retry_counter = 0;
    while (!success && retry_counter <= MODBUS_MAX_RETRIES)
    {
        success = mmodbus_readHoldingRegister32i(device_address, Total_Apparent_Energy_Phase_A_register, &data_a);
        success &= mmodbus_readHoldingRegister32i(device_address, Total_Apparent_Energy_Phase_B_register, &data_b);
        success &= mmodbus_readHoldingRegister32i(device_address, Total_Apparent_Energy_Phase_C_register, &data_c);
        success &= mmodbus_readHoldingRegister16i(device_address, Apparent_Energy_Scale_Factor_register, &raw_scale);

        // Transform raw scale to signed integer
        scale = (int16_t)raw_scale;

        *apparent_energy_a = (int64_t)((int32_t)data_a * pow(10, (int16_t)scale + 3));
        *apparent_energy_b = (int64_t)((int32_t)data_b * pow(10, (int16_t)scale + 3));
        *apparent_energy_c = (int64_t)((int32_t)data_c * pow(10, (int16_t)scale + 3));
        retry_counter++;
        DPRINT("Attempt %d: Raw Data A: %d, Raw Data B: %d, Raw Data C: %d, Raw Scale: %d, Scale: %d, apparent_energy A: %d, apparent_energy B: %d, apparent_energy C: %d", 
        retry_counter + 1, data_a, data_b, data_c, raw_scale, scale, *apparent_energy_a, *apparent_energy_b, *apparent_energy_c);
    }
    return success;
}

bool acurev_get_voltage(int16_t *voltage_a, int16_t *voltage_b, int16_t *voltage_c)
{
    uint16_t data_a, data_b, data_c, raw_scale;
    int16_t scale;
    bool success = false;
    uint8_t retry_counter = 0;

    while (!success && retry_counter <= MODBUS_MAX_RETRIES)
    {
        success = mmodbus_readHoldingRegister16i(device_address, Voltage_Phase_A_register, &data_a);
        success &= mmodbus_readHoldingRegister16i(device_address, Voltage_Phase_B_register, &data_b);
        success &= mmodbus_readHoldingRegister16i(device_address, Voltage_Phase_C_register, &data_c);
        success &= mmodbus_readHoldingRegister16i(device_address, Voltage_Scale_Factor_register, &raw_scale);

        // Transform raw scale to signed integer
        scale = (int16_t)raw_scale;

        *voltage_a = (int16_t)(data_a * pow(10, scale));
        *voltage_b = (int16_t)(data_b * pow(10, scale));
        *voltage_c = (int16_t)(data_c * pow(10, scale));

        DPRINT("Attempt %d: Raw Data A: %d, Raw Data B: %d, Raw Data C: %d, Raw Scale: %d, Scale: %d, Voltage A: %d, Voltage B: %d, Voltage C: %d", 
               retry_counter + 1, data_a, data_b, data_c, raw_scale, scale, *voltage_a, *voltage_b, *voltage_c);

        retry_counter++;
    }

    return success;
}

bool acurev_get_current(int32_t *current_a, int32_t *current_b, int32_t *current_c)
{
    uint16_t data_a, data_b, data_c, scale;
    bool success = false;
    uint8_t retry_counter = 0;

    while (!success && retry_counter <= MODBUS_MAX_RETRIES)
    {
        success = mmodbus_readHoldingRegister16i(device_address, Current_Phase_A_register, &data_a);
        success &= mmodbus_readHoldingRegister16i(device_address, Current_Phase_B_register, &data_b);
        success &= mmodbus_readHoldingRegister16i(device_address, Current_Phase_C_register, &data_c);
        success &= mmodbus_readHoldingRegister16i(device_address, Current_Scale_Factor_register, &scale);

        *current_a = (int32_t)( (int16_t)data_a * pow(10, ((int16_t)scale) + 3));
        *current_b = (int32_t)( (int16_t)data_b * pow(10, ((int16_t)scale) + 3));
        *current_c = (int32_t)( (int16_t)data_c * pow(10, ((int16_t)scale) + 3));

        DPRINT("Attempt %d: Raw Data A: %d, Raw Data B: %d, Raw Data C: %d, Scale: %d, Current A: %d, Current B: %d, Current C: %d", 
               retry_counter + 1, data_a, data_b, data_c, scale, *current_a, *current_b, *current_c);


        retry_counter++;
    }

    return success;
}

