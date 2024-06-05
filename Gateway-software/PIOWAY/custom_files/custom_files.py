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
        { "name":"Énergie apparente/Phase 1",         "dataType":"Long"},
        { "name":"Énergie apparente/Phase 2",         "dataType":"Long"},
        { "name":"Énergie apparente/Phase 3",         "dataType":"Long"},
        { "name":"Énergie active/Phase 1",            "dataType":"Long"},
        { "name":"Énergie active/Phase 2",            "dataType":"Long"},
        { "name":"Énergie active/Phase 3",            "dataType":"Long"},
        { "name":"Intensité/Phase 1",                 "dataType":"Integer"},
        { "name":"Intensité/Phase 2",                 "dataType":"Integer"},
        { "name":"Intensité/Phase 3",                 "dataType":"Integer"},
        { "name":"Tension/Phase 1",                   "dataType":"Short"},
        { "name":"Tension/Phase 2",                   "dataType":"Short"},
        { "name":"Tension/Phase 3",                   "dataType":"Short"},
        { "name":"Force du signal radio DASH7",       "dataType":"Short"},
        { "name":"Bouton pressé",                     "dataType":"Boolean"},
        { "name":"État de la liaison Modbus - DASH7", "dataType":"Boolean"},
      ]
    })

    def get_all_files(self):
        return sorted(self.files, key=lambda t: t.value)