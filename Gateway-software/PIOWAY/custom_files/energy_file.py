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
import datetime

from pyd7a.d7a.support.schema import Validatable, Types
from pyd7a.d7a.system_files.file import File
from .custom_file_ids import CustomFileIds


class EnergyFile(File, Validatable):
  FILE_SIZE = 17
  SCHEMA = [{
    "real_energy": Types.INTEGER(min=-0x8000000000000000, max=0x7FFFFFFFFFFFFFFF),  # int64
    "apparent_energy": Types.INTEGER(min=-0x8000000000000000, max=0x7FFFFFFFFFFFFFFF),  # int64
    "measurement_valid": Types.BOOLEAN()
  }]


  def __init__(self, real_energy=0, apparent_energy=0, measurement_valid=True):
    self.real_energy = real_energy
    self.apparent_energy = apparent_energy
    self.measurement_valid = measurement_valid
    File.__init__(self, CustomFileIds.ENERGY.value, self.FILE_SIZE)
    Validatable.__init__(self)

  @staticmethod
  def parse(s, offset=0, length=FILE_SIZE):
    real_energy = s.read("intle:64")
    apparent_energy = s.read("intle:64")
    measurement_valid = True if s.read("uint:8") else False
    return EnergyFile(real_energy=real_energy, apparent_energy=apparent_energy, measurement_valid=measurement_valid)
  
  def generate_home_assistant_data(self, device):
    mqtt_content = []
    dict = {'RealEnergy':self.real_energy, 'ApparentEnergy':self.apparent_energy, 'MeasurementValid':self.measurement_valid}
    for key, value in dict.items():
      content={}
      unique_id = '{}_{}'.format(device['ids'][0], key)
      component = 'binary_sensor' if isinstance(value, bool) else 'sensor'

      content['state_topic']='homeassistant/{}/{}/state'.format(component, unique_id)
      content['config_topic']='homeassistant/{}/{}/config'.format(component, unique_id)

      config = {
        'device': device,
        # 'icon': we could choose a custom icon
        # 'json_attributes_topic': ?
        'name': key,
        'qos': 1,
        'unique_id': unique_id,
        'state_topic': content['state_topic']
      }
      if component is 'sensor':
        config['dev_cla']= 'energy'
        config['stat_cla']= 'measurement'
        config['unit_of_meas']= 'Wh'
      
      content['config'] = json.dumps(config)

      if component is 'sensor':
        content['state'] = value
      else:
        content['state'] = 'ON' if value else 'OFF'

      mqtt_content.append(content)
    return mqtt_content

  def generate_mindsphere_data(self, link_budget):
    dict = {
      "timeseries": [
        {
          "timestamp":datetime.datetime.utcnow().strftime('%Y-%m-%dT%H:%M:%SZ'),
          "values":[
            {
              "dataPointId":"realEnergy",
              "value":self.real_energy,
              "qualityCode": "0"
            },
            {
              "dataPointId":"apparentEnergy",
              "value":self.apparent_energy,
              "qualityCode": "0"
            },
            {
              "dataPointId":"validEnergy",
              "value": "true" if self.measurement_valid else "false",
              "qualityCode": "0"
            },
            {
              "dataPointId":"dataPoint4",
              "value":link_budget,
              "qualityCode": "0"
            }
          ]
        }
      ]
    }
    return json.dumps(dict)

  def __iter__(self):
    for byte in bytearray(struct.pack(">q", self.real_energy)):  # big endian long long
      yield byte
    for byte in bytearray(struct.pack(">q", self.apparent_energy)):  # big endian long long
      yield byte
    yield self.measurement_valid


  def __str__(self):
    return "real_energy={}, apparent_energy={}, measurement_valid={}".format(
      self.real_energy, self.apparent_energy, self.measurement_valid
    )

class EnergyConfigFile(File, Validatable):
  FILE_SIZE = 17
  SCHEMA = [{
    "interval": Types.INTEGER(min=-0, max=0xFFFFFFFF),  # uint32
    "enabled": Types.BOOLEAN()
  }]
  component = 'sensor'


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
  
  def generate_home_assistant_data(self, device):
    mqtt_content = []
    dict = {'Interval':self.interval, 'Enabled':self.enabled}
    for key, value in dict.items():
      content={}
      unique_id = '{}_{}'.format(device['ids'][0], key)
      component = 'binary_sensor' if isinstance(value, bool) else 'sensor'

      content['state_topic']='homeassistant/{}/{}/state'.format(component, unique_id)
      content['config_topic']='homeassistant/{}/{}/config'.format(component, unique_id)

      config = {
        'device': device,
        # 'icon': we could choose a custom icon
        # 'json_attributes_topic': ?
        'name': key,
        'qos': 1,
        'unique_id': unique_id,
        'state_topic': content['state_topic']
      }
      if component is 'sensor':
        config['dev_cla']= 'duration'
        config['unit_of_meas']= 's'
        config['ent_cat']= 'diagnostic'
      
      content['config'] = json.dumps(config)

      if component is 'sensor':
        content['state'] = value
      else:
        content['state'] = 'ON' if value else 'OFF'

      mqtt_content.append(content)
    return mqtt_content

  def generate_mindsphere_data(self, link_budget):
    return None

  def __iter__(self):
    for byte in bytearray(struct.pack(">I", self.interval)):
      yield byte
    yield self.enabled


  def __str__(self):
    return "interval={}, enabled={}".format(
      self.interval, self.enabled
    )
