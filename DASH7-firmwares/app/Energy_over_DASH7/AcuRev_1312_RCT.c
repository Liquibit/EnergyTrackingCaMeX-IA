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
#include "hwsystem.h"

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

#define Meter_Data_Reset_register 525  
#define Communication_Revise_Operation_Authority_register 522 //0X02 : Meter Reset, Event Reset, Write Energy Data
#define password_register 523 //default password 0
#define new_password_register 524 //default password 0

void acurev_1312_rct_init()
{
    uart = uart_init(0, 19200, 0);
    uart_enable(uart);
    uart_set_rx_interrupt_callback(uart, &modbus_callback_stack);
    uart_rx_interrupt_enable(uart);
    
    mmodbus_init(modbus_timeout);
    mmodbus_set32bitOrder(MModBus_32bitOrder_CDAB);
    DPRINT("acurev inited");
    // acurev_reset_meter_record();
}

bool acurev_get_real_energy(int64_t *real_energy_a, int64_t *real_energy_b, int64_t *real_energy_c)
{
    uint32_t data[4];  // Array to store all 32-bit register values (3 energy registers + 1 scale factor register)
    uint16_t raw_scale;
    int16_t scale;
    bool success = false;
    uint8_t retry_counter = 0;
    while (!success && retry_counter <= MODBUS_MAX_RETRIES)
    {
        // Read all registers in a single call
        success = mmodbus_readHoldingRegisters32i(device_address, Total_Real_Energy_Phase_A_register, 4, data);
        
        if (success)
        {
            raw_scale = data[3] >> 16;
            scale = (int16_t)raw_scale;

            *real_energy_a = (int64_t)((int32_t)data[0] * pow(10, (int16_t)scale + 3));
            *real_energy_b = (int64_t)((int32_t)data[1] * pow(10, (int16_t)scale + 3));
            *real_energy_c = (int64_t)((int32_t)data[2] * pow(10, (int16_t)scale + 3));
        }
        else
        {
            hw_busy_wait(3000);
        }

        retry_counter++;
        DPRINT("Attempt %d: Raw Data A: %d, Raw Data B: %d, Raw Data C: %d, Raw Scale: %d, Scale: %d, real_energy A: %d, real_energy B: %d, real_energy C: %d", 
        retry_counter, data[0], data[1], data[2], data[3], scale, (int32_t)*real_energy_a, (int32_t)*real_energy_b, (int32_t)*real_energy_c); 
    }
    return success;
}


bool acurev_get_apparent_energy(int64_t *apparent_energy_a, int64_t *apparent_energy_b, int64_t *apparent_energy_c)
{
    uint32_t data[4];  // Array to store all 32-bit register values (3 energy registers + 1 scale factor register)
    int16_t scale;
    uint16_t raw_scale;
    bool success = false;
    uint8_t retry_counter = 0;
    while (!success && retry_counter <= MODBUS_MAX_RETRIES)
    {
        // Read all registers in a single call
        success = mmodbus_readHoldingRegisters32i(device_address, Total_Apparent_Energy_Phase_A_register, 4, data);
        
        if (success)
        {
            // Transform raw scale to signed integer
            raw_scale = data[3] >> 16;
            scale = (int16_t)raw_scale;

            *apparent_energy_a = (int64_t)((int32_t)data[0] * pow(10, (int16_t)scale + 3));
            *apparent_energy_b = (int64_t)((int32_t)data[1] * pow(10, (int16_t)scale + 3));
            *apparent_energy_c = (int64_t)((int32_t)data[2] * pow(10, (int16_t)scale + 3));
        }
        else
        {
            hw_busy_wait(3000);
        }


        retry_counter++;
        DPRINT("Attempt %d: Raw Data A: %d, Raw Data B: %d, Raw Data C: %d, Raw Scale: %d, Scale: %d, apparent_energy A: %d, apparent_energy B: %d, apparent_energy C: %d", 
        retry_counter, data[0], data[1], data[2], data[3], scale, (int32_t)*apparent_energy_a, (int32_t)*apparent_energy_b, (int32_t)*apparent_energy_c); 
    }
    return success;
}


bool acurev_get_voltage(int16_t *voltage_a, int16_t *voltage_b, int16_t *voltage_c)
{
    uint16_t data[8];  // Array to store all 16-bit register values (3 voltage registers + 4 unused registers + 1 scale register)
    int16_t scale;
    bool success = false;
    uint8_t retry_counter = 0;

    while (!success && retry_counter <= MODBUS_MAX_RETRIES)
    {
        // Read voltage registers and the scale register together
        success = mmodbus_readHoldingRegisters16i(device_address, Voltage_Phase_A_register, 8, data);
        
        if (success)
        {
            // Transform raw scale to signed integer
            scale = (int16_t)data[7];

            *voltage_a = (int16_t)(data[0] * pow(10, scale));
            *voltage_b = (int16_t)(data[1] * pow(10, scale));
            *voltage_c = (int16_t)(data[2] * pow(10, scale));

            DPRINT("Attempt %d: Raw Data A: %d, Raw Data B: %d, Raw Data C: %d, Raw Scale: %d, Scale: %d, Voltage A: %d, Voltage B: %d, Voltage C: %d", 
                   retry_counter + 1, data[0], data[1], data[2], data[7], scale, *voltage_a, *voltage_b, *voltage_c);
        }
        else
        {
            hw_busy_wait(3000);
        }


        retry_counter++;
    }

    return success;
}


bool acurev_get_current(int32_t *current_a, int32_t *current_b, int32_t *current_c)
{
    uint16_t data[4];  // Array to store all 16-bit register values (3 current registers + 1 scale register)
    int16_t scale;
    bool success = false;
    uint8_t retry_counter = 0;

    while (!success && retry_counter <= MODBUS_MAX_RETRIES)
    {
        // Read current registers and the scale register together
        success = mmodbus_readHoldingRegisters16i(device_address, Current_Phase_A_register, 4, data);
        
        if (success)
        {
            // Transform raw scale to signed integer
            scale = (int16_t)data[3];

            *current_a = (int32_t)( (int16_t)data[0] * pow(10, scale + 3));
            *current_b = (int32_t)( (int16_t)data[1] * pow(10, scale + 3));
            *current_c = (int32_t)( (int16_t)data[2] * pow(10, scale + 3));

            DPRINT("Attempt %d: Raw Data A: %d, Raw Data B: %d, Raw Data C: %d, raw Scale: %d, Scale: %d, Current A: %d, Current B: %d, Current C: %d", 
                   retry_counter + 1, data[0], data[1], data[2], data[3], scale, *current_a, *current_b, *current_c);
        }
        else
        {
            hw_busy_wait(3000);
        }
        retry_counter++;
    }

    return success;
}

bool acurev_gain_write_permission()
{
    bool success = true;
    uint16_t data[] = {0x02, 0, 0,0};
    success= mmodbus_writeHoldingRegisters16i_length2(device_address, Communication_Revise_Operation_Authority_register, data);
    log_print_string("1written reset register %d", success);
    return success;
}

bool acurev_reset_meter_record()
{
    bool success = true; 
    uint16_t data2[] = {0,0xFF}; // reset all data
    success= mmodbus_writeHoldingRegisters16i_length2(device_address, new_password_register, data2);
    log_print_string("2written reset register %d", success);
    return success;
}