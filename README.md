# EnergyTrackingCaMeX-IA
Energy tracking project for CaMéX-IA

This project exists of 4 parts:
- Energy Measurement device
- MODBUS-to-DASH7 device
- DASH7 gateway
- Gateway software to Scorpio

We will go over each part in this overview.

## Energy Measurement device

For this project, we used the Accuenergy AcuRev 1312-RCT-X0 together with 3 Rogowski coils. The official manual can be found [on their official website](https://www.accuenergy.com/wp-content/uploads/AcuRev-1312-User-Manual-Canada-Measurements-Approved.pdf). This device then measures the current over the three different phases and combines them. At the moment, only apparent and real energy are used in the rest of the project.

## MODBUS-to-DASH7 device

Custom hardware made by LiQuiBit. Every 10 minutes, it communicates over RS485 with the energy measurement device and sends this data over DASH7. 

The data gets structured as a file in the following format:

EnergyFile:
|Field|Type|
|---|---|
|apparent energy/phase 1|signed int 64|
|apparent energy/phase 2|signed int 64|
|apparent energy/phase 3|signed int 64|
|active energy/phase 1|signed int 64|
|active energy/phase 2|signed int 64|
|active energy/phase 3|signed int 64|
|Current/phase 1|signed int 32|
|Current/phase 2|signed int 32|
|Current/phase 3|signed int 32|
|Voltage/phase 1|signed int 16|
|Voltage/phase 2|signed int 16|
|Voltage/phase 3|signed int 16|
|Received Signal Strength|signed int 16|
|valid measurement|boolean|

Valid measurement indicates if it succeeded at reading out the values from the measurement device. 

You can find the firmware for this device in the DASH7-firmwares folder. 

For instructions on how to build or modify the application, you can take a look at [the LiQuiBit documentation](https://docs.liquibit.be/docs/Sub-iot/).

## DASH7 gateway

Using the [IOWAY gateway](https://www.liquibit.be/our_product.html) from LiQuiBit. Using the jumpers, the module is configured to forward DASH7 messages directly over UART instead of sending it to the ESP32. 

The firmware on the IOWAY gateway is the default IOWAY gateway application of the Sub-IoT stack. More information can be found in [the LiQuiBit documentation](https://docs.liquibit.be/docs/The-IOWAY-firmware/).

## Gateway software to Scorp-IO

For this project, this software was deployed on a raspberry pi. All necessary parts can be found in the Gateway-software folder.

The software is written in python3 as it's fully dependant on [the PyD7A library](https://github.com/liquibit/pyd7a). It is configured to receive messages from the serial connection, parse them into known files and forward it over MQTT to Scorp-IO. This sends all configuration to Scorp-IO on the first message of a device and does not require any extra configuration up front.

## Setting up the OS

First pull the code from the public github and put all aspects in the appropriate locations.
```
sudo git clone https://github.com/Liquibit/EnergyTrackingCaMeX-IA.git --recurse-submodules /opt/EnergyTrackingCaMeX-IA
sudo chmod +x /opt/EnergyTrackingCaMeX-IA/Gateway-software/os-config/gateway
sudo ln -s /opt/EnergyTrackingCaMeX-IA/Gateway-software/os-config/gateway /bin/gateway
sudo ln -s /opt/EnergyTrackingCaMeX-IA/Gateway-software/os-config/gateway.monit /etc/monit/conf.d/
sudo chown root:root /opt/EnergyTrackingCaMeX-IA/Gateway-software/os-config/PIOWAY
sudo ln -s /opt/EnergyTrackingCaMeX-IA/Gateway-software/os-config/PIOWAY /etc/logrotate.d/
```

This will set up the following things:
- Monit will make sure the code keeps running, restarts it when something goes wrong and starts it up on boot.
- Logrotate will rotate and compress the logs every week to make sure we're not using too much memory.

Then, make sure your python3 instance has all requirements installed
```
sudo python3 -m pip install -r /opt/EnergyTrackingCaMeX-IA/Gateway-software/pyd7a/requirements.txt
```

Now, you should be good to go after a reboot!
