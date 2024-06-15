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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "button.h"
#include "button_file.h"
#include "debug.h"
#include "led.h"
#include "little_queue.h"
#include "log.h"
#include "scheduler.h"
#include "energy_file.h"
#include "d7ap_fs.h"

#define FRAMEWORK_APP_LOG 1
#ifdef FRAMEWORK_APP_LOG
#include "log.h"
#define DPRINT(...) log_print_string(__VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif
static void userbutton_callback(uint8_t button_id, uint8_t mask, buttons_state_t buttons_state);


static void userbutton_callback(uint8_t button_id, uint8_t mask, buttons_state_t buttons_state)
{
    log_print_string("pressed\n");
}


/**
 * @brief Start of the application software
 */
void bootstrap()
{
    // initialize the network queue
    little_queue_init();
    button_file_register_cb(&userbutton_callback);
    button_files_initialize();
    button_file_set_measure_state(true);
    energy_files_initialize();
    energy_file_set_measure_state(true);

    led_flash(1);

    uint8_t uid[8];
    d7ap_fs_read_uid(uid);
    log_print_string("UID %02X%02X%02X%02X%02X%02X%02X%02X\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]);

    buttons_state_t booted_button_state = button_get_booted_state();

    if(booted_button_state == BUTTON1_PRESSED)
    {
        log_print_string("button pressed on boot, resetting energy meter data");
        energy_file_reset_accumulated_energy_data();
    }
    log_print_string("Device booted\n");

}
