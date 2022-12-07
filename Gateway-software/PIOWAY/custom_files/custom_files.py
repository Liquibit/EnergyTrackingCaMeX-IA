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

    def get_all_files(self):
        return sorted(self.files, key=lambda t: t.value)