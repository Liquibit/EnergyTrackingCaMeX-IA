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
#include "energy_file.h"
#include "d7ap_fs.h"
#include "errors.h"
#include "little_queue.h"
#include "log.h"
#include "network_manager.h"
#include "stdint.h"
#include "timer.h"
#include "AcuRev_1312_RCT.h"

#ifdef true
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

#define ENERGY_FILE_ID 52
#define ENERGY_FILE_SIZE sizeof(energy_file_t)
#define RAW_ENERGY_FILE_SIZE 67

#define ENERGY_CONFIG_FILE_ID 62
#define ENERGY_CONFIG_FILE_SIZE sizeof(energy_config_file_t)
#define RAW_ENERGY_CONFIG_FILE_SIZE 5

typedef struct {
    union {
        uint8_t bytes[RAW_ENERGY_FILE_SIZE];
        struct {
            int64_t apparent_energy_a;
            int64_t apparent_energy_b;
            int64_t apparent_energy_c;
            int64_t real_energy_a;
            int64_t real_energy_b;
            int64_t real_energy_c;
            int32_t current_a;
            int32_t current_b;
            int32_t current_c;
            int16_t voltage_a;
            int16_t voltage_b;
            int16_t voltage_c;
            bool measurement_valid;
        } __attribute__((__packed__));
    };
} energy_file_t;

typedef struct {
    union {
        uint8_t bytes[RAW_ENERGY_CONFIG_FILE_SIZE];
        struct {
            uint32_t interval;
            bool enabled;
        } __attribute__((__packed__));
    };
} energy_config_file_t;

static energy_file_t energy_file;

static void file_modified_callback(uint8_t file_id);
void energy_file_execute_measurement();
void measure_acurev_data();

static energy_config_file_t energy_config_file_cached
    = (energy_config_file_t) { .interval = 10 * 60, .enabled = true };

static bool energy_file_transmit_state = false;
static bool energy_config_file_transmit_state = false;

typedef enum {
    MEASURING_REAL_ENERGY = 0,
    MEASURING_APPARENT_ENERGY = 1,
    MEASURING_VOLTAGE = 2,
    MEASURING_CURRENT = 3,
    MEASURING_DONE = 4,
} measurement_state_t;

static measurement_state_t measurement_state = MEASURING_DONE;
bool current_measurement_valid = true;



/**
 * @brief Initialize the energy file and energy config file
 * The energy file tells us about the energy consumption of the connected installation, it includes the real energy and apparent energy
 * the energy config file configures the transmission interval and if the file is supposed to be send when a measruement occurs
 * @return error_t
 */
error_t energy_files_initialize()
{
    d7ap_fs_file_header_t volatile_file_header
        = { .file_permissions = (file_permission_t) { .guest_read = true, .user_read = true },
              .file_properties.storage_class = FS_STORAGE_VOLATILE,
              .length = ENERGY_FILE_SIZE,
              .allocated_length = ENERGY_FILE_SIZE };

    d7ap_fs_file_header_t permanent_file_header = { .file_permissions
        = (file_permission_t) { .guest_read = true, .guest_write = true, .user_read = true, .user_write = true },
        .file_properties.storage_class = FS_STORAGE_PERMANENT,
        .length = ENERGY_CONFIG_FILE_SIZE,
        .allocated_length = ENERGY_CONFIG_FILE_SIZE + 10 };

    uint32_t length = ENERGY_CONFIG_FILE_SIZE;
    error_t ret
        = d7ap_fs_read_file(ENERGY_CONFIG_FILE_ID, 0, energy_config_file_cached.bytes, &length, ROOT_AUTH);
    if (ret == -ENOENT) {
        ret = d7ap_fs_init_file(
            ENERGY_CONFIG_FILE_ID, &permanent_file_header, energy_config_file_cached.bytes);
        if (ret != SUCCESS) {
            log_print_error_string("Error initializing energy configuration file: %d", ret);
            return ret;
        }
    } else if (ret != SUCCESS)
        log_print_error_string("Error reading energy configuration file: %d", ret);

    ret = d7ap_fs_init_file(ENERGY_FILE_ID, &volatile_file_header, energy_file.bytes);
    if (ret != SUCCESS) {
        log_print_error_string("Error initializing energy file: %d", ret);
    }

    acurev_1312_rct_init(); //init the energy measurement device

    // set the configurations of the configuration file and register a callback on all changes on those files
    d7ap_fs_register_file_modified_callback(ENERGY_CONFIG_FILE_ID, &file_modified_callback);
    d7ap_fs_register_file_modified_callback(ENERGY_FILE_ID, &file_modified_callback);
    sched_register_task(&energy_file_execute_measurement);
    sched_register_task(&measure_acurev_data);
    DPRINT("energy file inited");
    return ret;
}

static void file_modified_callback(uint8_t file_id)
{
    if (file_id == ENERGY_CONFIG_FILE_ID) {
        // energy config file got modified
        uint32_t size = ENERGY_CONFIG_FILE_SIZE;
        d7ap_fs_read_file(ENERGY_CONFIG_FILE_ID, 0, energy_config_file_cached.bytes, &size, ROOT_AUTH);
        // set a timer to read the energy periodically
        if (energy_config_file_cached.enabled && energy_file_transmit_state)
            timer_post_task_delay(
                &energy_file_execute_measurement, energy_config_file_cached.interval * TIMER_TICKS_PER_SEC);
        else
            timer_cancel_task(&energy_file_execute_measurement);

        if (energy_config_file_transmit_state)
            queue_add_file(
                energy_config_file_cached.bytes, ENERGY_CONFIG_FILE_SIZE, ENERGY_CONFIG_FILE_ID);
    } else if (file_id == ENERGY_FILE_ID) {
        // energy file got modified, most likely internally
        uint32_t size = ENERGY_FILE_SIZE;
        d7ap_fs_read_file(ENERGY_FILE_ID, 0, energy_file.bytes, &size, ROOT_AUTH);
        queue_add_file(energy_file.bytes, ENERGY_FILE_SIZE, ENERGY_FILE_ID);
        timer_post_task_delay(
            &energy_file_execute_measurement, energy_config_file_cached.interval * TIMER_TICKS_PER_SEC);
    }
}

void energy_file_transmit_config_file()
{
    uint32_t size = ENERGY_CONFIG_FILE_SIZE;
    d7ap_fs_read_file(ENERGY_CONFIG_FILE_ID, 0, energy_config_file_cached.bytes, &size, ROOT_AUTH);
    queue_add_file(energy_config_file_cached.bytes, ENERGY_CONFIG_FILE_SIZE, ENERGY_CONFIG_FILE_ID);
}

void energy_file_execute_measurement()
{
    measurement_state = MEASURING_REAL_ENERGY;
    current_measurement_valid = true;
    measure_acurev_data();
}

void measure_acurev_data()
{
    DPRINT("executing energy measurement");
    
    if(measurement_state == MEASURING_REAL_ENERGY)
    {
        current_measurement_valid &= acurev_get_real_energy(&energy_file.real_energy_a, &energy_file.real_energy_b, &energy_file.real_energy_c);
        measurement_state = MEASURING_APPARENT_ENERGY;
        timer_post_task_delay(&measure_acurev_data, 50);
    }
    else if(measurement_state == MEASURING_APPARENT_ENERGY)
    {
        current_measurement_valid &= acurev_get_apparent_energy(&energy_file.apparent_energy_a, &energy_file.apparent_energy_b, &energy_file.apparent_energy_c);
        measurement_state = MEASURING_VOLTAGE;
        timer_post_task_delay(&measure_acurev_data, 50);
    }
    else if(measurement_state == MEASURING_VOLTAGE)
    {
        current_measurement_valid &= acurev_get_voltage(&energy_file.voltage_a, &energy_file.voltage_b, &energy_file.voltage_c);
        measurement_state = MEASURING_CURRENT;
        timer_post_task_delay(&measure_acurev_data, 50);
    }
    else if(measurement_state == MEASURING_CURRENT)
    {
        current_measurement_valid &= acurev_get_current(&energy_file.current_a, &energy_file.current_b, &energy_file.current_c);
        energy_file.measurement_valid = current_measurement_valid;
        d7ap_fs_write_file(ENERGY_FILE_ID, 0, energy_file.bytes, sizeof(energy_file), ROOT_AUTH);
        measurement_state = MEASURING_DONE;
    }
}


void energy_file_set_measure_state(bool enable)
{
    // enable or disable the periodic voltage measurement
    timer_cancel_task(&energy_file_execute_measurement);
    energy_file_transmit_state = enable;
    energy_config_file_transmit_state = enable;
    if (energy_config_file_cached.enabled && energy_file_transmit_state)
        timer_post_task_delay(
            &energy_file_execute_measurement, energy_config_file_cached.interval * TIMER_TICKS_PER_SEC);
}


void energy_file_set_enabled(bool enable)
{
    // enable or disable sending and gathering the energy file
    if (energy_config_file_cached.enabled != enable) {
        energy_config_file_cached.enabled = enable;
        d7ap_fs_write_file(ENERGY_CONFIG_FILE_ID, 0, energy_config_file_cached.bytes,
            ENERGY_CONFIG_FILE_SIZE, ROOT_AUTH);
    }
}

void energy_file_set_interval(uint32_t interval)
{
    // change the interval on which the energy file gets gathered and sent
    if (energy_config_file_cached.interval != interval) {
        energy_config_file_cached.interval = interval;
        d7ap_fs_write_file(ENERGY_CONFIG_FILE_ID, 0, energy_config_file_cached.bytes,
            ENERGY_CONFIG_FILE_SIZE, ROOT_AUTH);
    }
}

void energy_file_reset_accumulated_energy_data()
{
    timer_post_task_delay(&acurev_gain_write_permission, 5 * TIMER_TICKS_PER_SEC);
    timer_post_task_delay(&acurev_reset_meter_record, 7 * TIMER_TICKS_PER_SEC);
}