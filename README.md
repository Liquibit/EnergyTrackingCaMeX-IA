# EnergyTrackingCaMeX-IA
Energy tracking project for CaMéX-IA

This project exists of 4 parts:
- Energy Measurement device
- MODBUS-to-DASH7 device
- DASH7 gateway
- Gateway software to Mindsphere

We will go over each part in this overview.

## Energy Measurement device

For this project, we used the Accuenergy AcuRev 1312-RCT-X0 together with 3 Rogowski coils. The official manual can be found [on their official website](https://www.accuenergy.com/wp-content/uploads/AcuRev-1312-User-Manual-Canada-Measurements-Approved.pdf). This device then measures the current over the three different phases and combines them. At the moment, only apparent and real energy are used in the rest of the project.

## MODBUS-to-DASH7 device

Custom hardware made by LiQuiBit. Every 15 minutes, it communicates over RS485 with the energy measurement device and sends this data over DASH7. 

The data gets structured as a file in the following format:

EnergyFile:
|Field|Type|
|---|---|
|real energy|signed int 64|
|apparent energy|signed int 64|
|valid energy|boolean|

Valid energy indicates if it succeeded at reading out the values from the measurement device. 

You can find the firmware for this device in the DASH7-firmwares folder. 

For instructions on how to build or modify the application, you can take a look at [the LiQuiBit documentation](https://docs.liquibit.be/docs/Sub-iot/).

## DASH7 gateway

Using the [IOWAY gateway](https://www.liquibit.be/our_product.html) from LiQuiBit. Using the jumpers, the module is configured to forward DASH7 messages directly over UART instead of sending it to the ESP32. 

The firmware on the IOWAY gateway is the default IOWAY gateway application of the Sub-IoT stack. More information can be found in [the LiQuiBit documentation](https://docs.liquibit.be/docs/The-IOWAY-firmware/).

## Gateway software to Mindsphere

For this project, this software was deployed on a raspberry pi. All necessary parts can be found in the Gateway-software folder.

The software is written in python2 as it's fully dependant on [the PyD7A library](https://github.com/liquibit/pyd7a). It is configured to receive messages from the serial connection, parse them into known files and forward it over MQTT to a mindpshere instance. This requires an aspect to already be configured and initialised in Mindsphere.

The software is set up with Monit to start on boot and make sure it keeps running. An example can be found in Gateway-software/monit.
