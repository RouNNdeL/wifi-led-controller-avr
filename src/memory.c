#include <memory.h>
#include <avr/eeprom.h>

device_settings EEMEM settings_addr[DEVICE_COUNT];

void save_settings(device_settings *settings, uint8_t device_index) {
    eeprom_update_block(settings, &settings_addr[device_index], sizeof(device_settings));
}

void read_settings(device_settings *settings, uint8_t device_index) {
    eeprom_read_block(settings, &settings_addr[device_index], sizeof(device_settings));
}

/**
 * Reads all device settings from EEPROM to an array staring at {@code settings}
 * @param settings starting address of the {@code device_settings}' array
 * @param device_count number of device to read
 */
void read_all_settings(device_settings settings[DEVICE_COUNT]) {
    eeprom_read_block(settings, &settings_addr, sizeof(device_settings) * DEVICE_COUNT);
}