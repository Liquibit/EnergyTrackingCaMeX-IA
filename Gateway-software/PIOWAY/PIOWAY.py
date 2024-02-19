#!/usr/bin/env python
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

import argparse

import logging
import platform
import signal
import sys
import traceback

import time
import json

from custom_files.custom_files import CustomFiles
from custom_files.energy_file import EnergyFile, EnergyConfigFile
from custom_files.button_file import ButtonFile, ButtonConfigFile

import paho.mqtt.client as mqtt
import ssl

from pyd7a.modem.modem import Modem
from pyd7a.util.logger import configure_default_logger


class Modem2Mqtt():

  def __init__(self):
    argparser = argparse.ArgumentParser()
    argparser.add_argument("-d", "--device", help="serial device /dev file modem",
                           default="/dev/ttyACM0")
    argparser.add_argument("-r", "--rate", help="baudrate for serial device", type=int, default=115200)
    argparser.add_argument("-v", "--verbose", help="verbose", default=False, action="store_true")
    argparser.add_argument("-b", "--broker", help="mqtt broker hostname", default="homeassistant.local")
    argparser.add_argument("-u", "--user", help="mqtt username", default="")
    argparser.add_argument("-p", "--password", help="mqtt password", default="")
    argparser.add_argument("-l", "--log", help="path to the log file", default="/var/log/gateway.log")
    argparser.add_argument("-ca", "--ca-cert", help="path to CA file", default=None)
    argparser.add_argument("--cl-cert", help="path to the client certificate file", default=None)
    argparser.add_argument("--key-file", help="path to client key file", default=None)
    argparser.add_argument("-c", "--client-id", help="client id used for the mqtt client", default="")
    argparser.add_argument("-pr", "--project-id", help="project id used in the mqtt topic", default="")
    argparser.add_argument("-e", "--edge-node-id", help="edge node id used in the mqtt topic", default="")

    self.known_transmitters = [] # make sure to only transmit config once

    self.config = argparser.parse_args()
    configure_default_logger(self.config.verbose, file=self.config.log)

    self.modem = Modem(self.config.device, self.config.rate, self.on_command_received, custom_files_class=CustomFiles)
    self.modem.connect()
    self.connect_to_mqtt()


  def connect_to_mqtt(self):
    self.connected_to_mqtt = False

    self.mq = mqtt.Client(self.config.client_id, True, None, mqtt.MQTTv311)
    self.mq.on_connect = self.on_mqtt_connect
    self.mq.on_publish = self.on_published
    self.mq.on_disconnect = self.on_mqtt_disconnect
    # self.mq.on_message = self.on_mqtt_message
    if self.config.ca_cert and self.config.cl_cert and self.config.key_file:
      port = 8883
      self.mq.tls_set(ca_certs=self.config.ca_cert, certfile=self.config.cl_cert, keyfile=self.config.key_file, tls_version=ssl.PROTOCOL_TLSv1_2)
      self.mq.tls_insecure_set(True)
    else:
      port = 1883

    self.mq.username_pw_set(self.config.user, self.config.password)

    self.mq.connect_async(self.config.broker, port)
    self.mq.loop_start()
    while not self.connected_to_mqtt: pass  # busy wait until connected
    logging.info("Connected to MQTT broker on {}".format(
      self.config.broker
    ))

  def on_mqtt_connect(self, client, config, flags, rc):
    logging.info("mqtt connected")
    # self.mq.subscribe(self.mqtt_topic_outgoing)
    self.connected_to_mqtt = True
    
  def on_mqtt_disconnect(self, client, userdata, rc):
    logging.warning("mqtt disconnected: {}".format(mqtt.connack_string(rc)))

  def on_mqtt_message(self, client, config, msg):
    # downlink is currently not handled yet
    logging.info("gotten downlink {}".format(msg.payload))

  def __del__(self): # pragma: no cover
    try:
      self.mq.loop_stop()
      self.mq.disconnect()
      self.modem.stop_reading()
    except: pass

  def on_published(self, client, userdata, mid):
    logging.info("published message with id {} successfully".format(mid))

  def on_command_received(self, cmd):
    try:
      transmitter = cmd.interface_status.operand.interface_status.addressee.id
      link_budget = cmd.interface_status.operand.interface_status.link_budget
      transmitterHexString = hex(transmitter)[2:-1]
      operation = cmd.actions[0].operation
      if operation.file_type is None or operation.file_data_parsed is None:
        logging.info("received random data: {} from {}".format(operation.operand.data, transmitterHexString))
        return
      fileType = operation.file_type
      parsedData = operation.file_data_parsed
      logging.info("Received {} content: {} from {}".format(fileType.__class__.__name__,
                                              parsedData, transmitterHexString))

      if fileType.__class__ in [ButtonFile, ButtonConfigFile, EnergyFile, EnergyConfigFile]:
        config_json, data_json = parsedData.generate_scorp_io_data(link_budget)
        
        # if we did not send anything for this end device yet, first send the config
        if transmitter not in self.known_transmitters:
          self.known_transmitters.append(transmitter)
          self.mq.publish(f"mqtts/{self.config.project_id}/DBIRTH/{self.config.edge_node_id}/{transmitterHexString}", config_json, qos=1, retain=True)
        
        self.mq.publish(f"mqtts/{self.config.project_id}/DDATA/{self.config.edge_node_id}/{transmitterHexString}", data_json, qos=1, retain=False)
        
        logging.info(f"published file for {transmitterHexString}")

    except (AttributeError, IndexError):
      # probably an answer on downlink we don't care about right now
      return
    except:
      exc_type, exc_value, exc_traceback = sys.exc_info()
      lines = traceback.format_exception(exc_type, exc_value, exc_traceback)
      trace = "".join(lines)
      logging.error("Exception while processing command: \n{}".format(trace))

  def run(self):
    logging.info("Started")
    keep_running = True
    while keep_running:
      try:
        if platform.system() == "Windows":
          time.sleep(1)
        else:
          signal.pause()
      except KeyboardInterrupt:
        self.__del__()
        logging.info("received KeyboardInterrupt... stopping processing")
        keep_running = False

if __name__ == "__main__":
  Modem2Mqtt().run()
