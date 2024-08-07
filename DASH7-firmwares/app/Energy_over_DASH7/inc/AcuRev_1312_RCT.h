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
#ifndef __ACUREF_1312_RCT_H
#define __ACUREF_1312_RCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"


void acurev_1312_rct_init();
bool acurev_get_real_energy(int64_t *real_energy_a, int64_t *real_energy_b, int64_t *real_energy_c);
bool acurev_get_apparent_energy(int64_t *apparent_energy_a, int64_t *apparent_energy_b, int64_t *apparent_energy_c);
bool acurev_get_voltage(int16_t *voltage_a, int16_t *voltage_b, int16_t *voltage_c);
bool acurev_get_current(int32_t *current_a, int32_t *current_b, int32_t *current_c);
void acurev_gain_write_permission();
void acurev_reset_meter_record();


#endif //__ACUREF_1312_RCT_H