import json

from .custom_file_ids import CustomFileIds

from .energy_file import EnergyFile, EnergyConfigFile
from .button_file import ButtonFile, ButtonConfigFile

class CustomFiles:
    enum_class = CustomFileIds

    files = {
        CustomFileIds.ENERGY: EnergyFile(),
        CustomFileIds.ENERGY_CONFIGURATION: EnergyConfigFile(),
        CustomFileIds.BUTTON: ButtonFile(),
        CustomFileIds.BUTTON_CONFIGURATION: ButtonConfigFile(),
    }

    global_sparkplug_config =  json.dumps({
      "metrics" : [
        { "name":"Énergie apparante/phase 1",         "dataType":"Long"},
        { "name":"Énergie apparante/phase 2",         "dataType":"Long"},
        { "name":"Énergie apparante/phase 3",         "dataType":"Long"},
        { "name":"Énergie active/phase 1",            "dataType":"Long"},
        { "name":"Énergie active/phase 2",            "dataType":"Long"},
        { "name":"Énergie active/phase 3",            "dataType":"Long"},
        { "name":"Courent/phase 1",                   "dataType":"Long"},
        { "name":"Courent/phase 2",                   "dataType":"Long"},
        { "name":"Courent/phase 3",                   "dataType":"Long"},
        { "name":"Voltage/phase 1",                   "dataType":"Short"},
        { "name":"Voltage/phase 2",                   "dataType":"Short"},
        { "name":"Voltage/phase 3",                   "dataType":"Short"},
        { "name":"Force du signal radio DASH7",       "dataType":"Short"},
        { "name":"Bouton Pressé",                     "dataType":"Boolean"},
        { "name":"État de la liaison Modbus - DASH7", "dataType":"Boolean"},
      ]
    })

    def get_all_files(self):
        return sorted(self.files, key=lambda t: t.value)