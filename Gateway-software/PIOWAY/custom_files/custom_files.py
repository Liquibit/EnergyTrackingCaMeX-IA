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

    global_sparkplug_config =  {
      "metrics" : [
        { "Name":"Énergie apparante/phase 1",       "dataType":"Long"},
        { "Name":"Énergie apparante/phase 2",       "dataType":"Long"},
        { "Name":"Énergie apparante/phase 3",       "dataType":"Long"},
        { "Name":"Énergie active/phase 1",          "dataType":"Long"},
        { "Name":"Énergie active/phase 2",          "dataType":"Long"},
        { "Name":"Énergie active/phase 3",          "dataType":"Long"},
        { "Name":"Courent/phase 1",                 "dataType":"Long"},
        { "Name":"Courent/phase 2",                 "dataType":"Long"},
        { "Name":"Courent/phase 3",                 "dataType":"Long"},
        { "Name":"Voltage/phase 1",                 "dataType":"Short"},
        { "Name":"Voltage/phase 2",                 "dataType":"Short"},
        { "Name":"Voltage/phase 3",                 "dataType":"Short"},
        { "Name":"Force du signal radio DASH7",     "dataType":"Short"},
        { "Name":"Bouton Pressé",                   "dataType":"Boolean"},
        { "Name":"État de la liaison Modbus/DASH7", "dataType":"Boolean"},
      ]
    }

    def get_all_files(self):
        return sorted(self.files, key=lambda t: t.value)