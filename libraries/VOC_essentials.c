//
// Created by save0 on 02/06/2025.
//

#include "VOC_essentials.h"

#include "sensirion_i2c_hal.h"

int16_t mux_port_select(uint8_t mux_addr, uint8_t mux_port) {
    if (mux_port > 7) return 1;
   uint8_t data = 1<<mux_port;
    sensirion_i2c_hal_write(mux_addr,&data, 1);
}

int16_t mux_i2c_detect(uint8_t mux_addr) {
    for (uint8_t addr = 0; addr<=127; addr++) {
        if (addr == mux_addr) continue;

        if (sensirion_i2c_hal_write(addr,NULL, 0) == 0) {
            return 1;
        }
        return 0;
    }
}

/* Check if needed, no longer have svm41 so maybe no offset? */
void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    strftime(buffer, size, "%Y-%m-%d_%H:%M:%S", localtime(&now));
}

int read_config(int* oversample_count, float* humidity_offset) {
    FILE* config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL) {
        fprintf(stderr, "Config file not found, using default values.\n");
        return -1; // Return error if file not found
    }

    char line[128];
    while (fgets(line, sizeof(line), config_file)) {
        char key[64], value[64];

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#') continue;

        // Parse key-value pairs
        if (sscanf(line, "%63s = %63s", key, value) == 2) {
            if (strcmp(key, "oversample_count") == 0) {
                *oversample_count = atoi(value);
                if (*oversample_count <= 0) {
                    fprintf(stderr, "Invalid oversample_count in config, using default value of 5.\n");
                    *oversample_count = 5;
                }
            } else if (strcmp(key, "humidity_offset") == 0) {
                *humidity_offset = atof(value);
                if (*humidity_offset < 0) {
                    fprintf(stderr, "Invalid humidity_offset in config, using default value of 2.8.\n");
                    *humidity_offset = 2.8;
                }
            }
        }
    }

    fclose(config_file);
    return 0; // Success
}
