#
# Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
#
# This file is part of pyd7a.
# See https://github.com/Sub-IoT/pyd7a for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import json
import time

from enum import Enum

from pyd7a.d7a.support.schema import Validatable, Types
from pyd7a.d7a.system_files.file import File
from .custom_file_ids import CustomFileIds

class ButtonStates(Enum):
    NO_BUTTON_PRESSED = 0
    BUTTON1_PRESSED = 1
    BUTTON2_PRESSED = 2
    BUTTON1_2_PRESSED = 3
    BUTTON3_PRESSED = 4
    BUTTON1_3_PRESSED = 5
    BUTTON2_3_PRESSED = 6
    ALL_BUTTONS_PRESSED = 7


class ButtonFile(File, Validatable):
  FILE_SIZE = 6
  SCHEMA = [{
    "button_id": Types.INTEGER(min=0, max=0xFF),
    "mask": Types.BOOLEAN(),
    "buttons_state": Types.INTEGER(min=0, max=0xFF)
  }]
  component = 'binary_sensor'


  def __init__(self, button_id=0, mask=False, buttons_state=0):
    self.button_id = button_id
    self.mask = mask
    self.buttons_state = buttons_state
    File.__init__(self, CustomFileIds.BUTTON.value, ButtonFile.FILE_SIZE)
    Validatable.__init__(self)

  @staticmethod
  def parse(s, offset=0, length=FILE_SIZE):
    button_id = s.read("uint:8")
    mask = True if s.read("uint:8") else False
    buttons_state = s.read("uint:8")
    return ButtonFile(button_id=button_id, mask=mask, buttons_state=buttons_state)

  def generate_scorp_io_data(self, link_budget):
    timestamp = round( time.time() * 1000 ) # get time in milliseconds
    data = {
      "metrics" : [
        { "name":"Bouton pressé",               "dataType":"Boolean",    "timestamp":timestamp, "value":(self.buttons_state & ButtonStates.BUTTON1_PRESSED.value) > 0 },
        { "name":"Force du signal radio DASH7", "dataType":"Short",      "timestamp":timestamp, "value":link_budget                                                   },
      ]
    }
    data_json = json.dumps(data)
    return data_json
  
  def __iter__(self):
    yield self.button_id
    yield self.mask
    yield self.buttons_state


  def __str__(self):
    return "button_id={}, mask={}, buttons_state={}".format(
      self.button_id, self.mask, self.buttons_state
    )
  
class ButtonConfigFile(File, Validatable):
  FILE_SIZE = 4
  SCHEMA = [{
    "transmit_mask_0": Types.BOOLEAN(),
    "transmit_mask_1": Types.BOOLEAN(),
    "button_control_menu": Types.BOOLEAN(),
    "enabled": Types.BOOLEAN()
  }]
  component = 'binary_sensor'


  def __init__(self, transmit_mask_0=True, transmit_mask_1=True, button_control_menu=True, enabled=True):
    self.transmit_mask_0 = transmit_mask_0
    self.transmit_mask_1 = transmit_mask_1
    self.button_control_menu = button_control_menu
    self.enabled = enabled
    File.__init__(self, CustomFileIds.BUTTON_CONFIGURATION.value, ButtonConfigFile.FILE_SIZE)
    Validatable.__init__(self)

  @staticmethod
  def parse(s, offset=0, length=FILE_SIZE):
    transmit_mask_0 = True if s.read("uint:8") else False
    transmit_mask_1 = True if s.read("uint:8") else False
    button_control_menu = True if s.read("uint:8") else False
    enabled = True if s.read("uint:8") else False
    return ButtonConfigFile(transmit_mask_0=transmit_mask_0, transmit_mask_1=transmit_mask_1, button_control_menu=button_control_menu, enabled=enabled)

  def generate_scorp_io_data(self, link_budget):
    return None
  
  def __iter__(self):
    yield self.transmit_mask_0
    yield self.transmit_mask_1
    yield self.button_control_menu
    yield self.enabled


  def __str__(self):
    return "transmit_mask_0={}, transmit_mask_1={}, button_control_menu={}, enabled={}".format(
      self.transmit_mask_0, self.transmit_mask_1, self.button_control_menu, self.enabled
    )