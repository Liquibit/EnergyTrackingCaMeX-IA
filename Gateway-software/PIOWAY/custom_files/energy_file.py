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
import struct
import json
import time

from pyd7a.d7a.support.schema import Validatable, Types
from pyd7a.d7a.system_files.file import File
from .custom_file_ids import CustomFileIds


class EnergyFile(File, Validatable):
  FILE_SIZE = 97
  SCHEMA = [{
    # "apparent_energy": Types.LIST(Types.INTEGER(min=-0x8000000000000000, max=0x7FFFFFFFFFFFFFFF)), # 3 phases int64
    # "real_energy": Types.LIST(Types.INTEGER(min=-0x8000000000000000, max=0x7FFFFFFFFFFFFFFF)), # 3 phases int64
    # "current": Types.LIST(Types.INTEGER(min=-0x8000000000000000, max=0x7FFFFFFFFFFFFFFF)), # 3 phases int64
    # "voltage": Types.LIST(Types.INTEGER(min=-0x8000, max=0x7FFF)), # 3 phases int16
    # "measurement_valid": Types.BOOLEAN()
  }]


  def __init__(self, real_energy=[], apparent_energy=[], current=[], voltage=[], measurement_valid=True):
    self.apparent_energy = apparent_energy
    self.real_energy = real_energy
    self.current = current
    self.voltage = voltage
    self.measurement_valid = measurement_valid
    File.__init__(self, CustomFileIds.ENERGY.value, self.FILE_SIZE)
    Validatable.__init__(self)

  @staticmethod
  def parse(s, offset=0, length=FILE_SIZE):
    apparent_energy = []
    real_energy = []
    current = []
    voltage = []

    for i in range(3):
      apparent_energy.append(s.read("intle:64"))
    for i in range(3):
      real_energy.append(s.read("intle:64"))
    for i in range(3):
      current.append(s.read("intle:64"))
    for i in range(3):
      voltage.append(s.read("intle:16"))
    measurement_valid = True if s.read("uint:8") else False

    return EnergyFile(real_energy=real_energy, apparent_energy=apparent_energy, current=current, voltage=voltage, measurement_valid=measurement_valid)
  
  def generate_scorp_io_data(self, link_budget):
    timestamp = round( time.time() * 1000 ) # get time in milliseconds
    data = {
      "metrics" : [
        { "name":"Énergie apparante/phase 1",         "dataType":"Long",    "timestamp":timestamp, "value":self.apparent_energy[0] },
        { "name":"Énergie apparante/phase 2",         "dataType":"Long",    "timestamp":timestamp, "value":self.apparent_energy[1] },
        { "name":"Énergie apparante/phase 3",         "dataType":"Long",    "timestamp":timestamp, "value":self.apparent_energy[2] },
        { "name":"Énergie active/phase 1",            "dataType":"Long",    "timestamp":timestamp, "value":self.real_energy[0]     },
        { "name":"Énergie active/phase 2",            "dataType":"Long",    "timestamp":timestamp, "value":self.real_energy[1]     },
        { "name":"Énergie active/phase 3",            "dataType":"Long",    "timestamp":timestamp, "value":self.real_energy[2]     },
        { "name":"Courent/phase 1",                   "dataType":"Long",    "timestamp":timestamp, "value":self.current[0]         },
        { "name":"Courent/phase 2",                   "dataType":"Long",    "timestamp":timestamp, "value":self.current[1]         },
        { "name":"Courent/phase 3",                   "dataType":"Long",    "timestamp":timestamp, "value":self.current[2]         },
        { "name":"Voltage/phase 1",                   "dataType":"Short",   "timestamp":timestamp, "value":self.voltage[0]         },
        { "name":"Voltage/phase 2",                   "dataType":"Short",   "timestamp":timestamp, "value":self.voltage[1]         },
        { "name":"Voltage/phase 3",                   "dataType":"Short",   "timestamp":timestamp, "value":self.voltage[2]         },
        { "name":"Force du signal radio DASH7",       "dataType":"Short",   "timestamp":timestamp, "value":link_budget             },
        { "name":"État de la liaison Modbus - DASH7", "dataType":"Boolean", "timestamp":timestamp, "value":self.measurement_valid  },
      ]
    }
    data_json = json.dumps(data)

    return data_json

  def __iter__(self):
    for i in range(3):
      for byte in bytearray(struct.pack(">q", self.real_energy[i])):  # big endian long long
        yield byte
    for i in range(3):
      for byte in bytearray(struct.pack(">q", self.apparent_energy[i])):  # big endian long long
        yield byte
    for i in range(3):
      for byte in bytearray(struct.pack(">q", self.current[i])):  # big endian long long
        yield byte
    for i in range(3):
      for byte in bytearray(struct.pack(">h", self.voltage[i])):  # big endian short
        yield byte
    yield self.measurement_valid


  def __str__(self):
    return "real_energy={}, apparent_energy={}, current={}, voltage={}, measurement_valid={}".format(
      self.real_energy, self.apparent_energy, self.current, self.voltage, self.measurement_valid
    )

class EnergyConfigFile(File, Validatable):
  FILE_SIZE = 5
  SCHEMA = [{
    "interval": Types.INTEGER(min=-0, max=0xFFFFFFFF),  # uint32
    "enabled": Types.BOOLEAN()
  }]

  def __init__(self, interval=0, enabled=True):
    self.interval = interval
    self.enabled = enabled
    File.__init__(self, CustomFileIds.ENERGY_CONFIGURATION.value, self.FILE_SIZE)
    Validatable.__init__(self)

  @staticmethod
  def parse(s, offset=0, length=FILE_SIZE):
    interval = s.read("uint:32")
    enabled = True if s.read("uint:8") else False
    return EnergyConfigFile(interval=interval, enabled=enabled)

  def generate_scorp_io_data(self, link_budget):
    return None, None

  def __iter__(self):
    for byte in bytearray(struct.pack(">I", self.interval)):
      yield byte
    yield self.enabled


  def __str__(self):
    return "interval={}, enabled={}".format(
      self.interval, self.enabled
    )
