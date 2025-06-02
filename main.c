#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>


#include "libraries/sensirion_common.h"
#include "libraries/sensirion_i2c.h"
#include "libraries/sensirion_i2c_hal.h"
#include "libraries/sgp40_i2c.h"
#include "libraries/sht3x_i2c.h"



#define LOG_DIR "../logs"
#define CONFIG_FILE "../config.txt"

/* Check if needed, no longer have svm41 so maybe no offset? */
void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    strftime(buffer, size, "%Y-%m-%d_%H:%M:%S", localtime(&now));
}

// Function to read config file and extract parameters
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


int main(int argc, char* argv[]) {

    char filename[128];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    // Format timestamp
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", t);

    if (argc >= 2) {
        snprintf(filename, sizeof(filename), "%s/%s_%s.csv", LOG_DIR, argv[1], timestamp);
    } else {
        snprintf(filename, sizeof(filename), "%s/log_%s.csv", LOG_DIR, timestamp);
    }

    int oversample_count = 5;  // Default to 5 samples (5 seconds)
    float sht31_humidity_offset = 0; // Default humidity offset

    // Read configuration from file
    if (read_config(&oversample_count, &sht31_humidity_offset) != 0) {
        printf("Using default configuration: oversample_count = %d, humidity_offset = %.2f\n", oversample_count, sht31_humidity_offset);
    } else {
        printf("Loaded configuration: oversample_count = %d, humidity_offset = %.2f\n", oversample_count, sht31_humidity_offset);
    }

    int16_t error = 0;

    sensirion_i2c_hal_init();

    sht3x_init(SHT31_I2C_ADDR_44);


    mkdir(LOG_DIR, 0755);  // Safe: will do nothing if already exists


    FILE *logfile = fopen(filename, "a");
    if (!logfile) {
        perror("Failed to open log file");
        return 1;
    }

    printf("Logging to file: %s\n", filename);

    // Write header only if file is new
    fseek(logfile, 0, SEEK_END);
    long size = ftell(logfile);
    if (size == 0) {
        fprintf(logfile, "Timestamp,Temperature_C_sht31,Humidity_pct_sht31,Raw_VOC_sgp40,Temperature_C_svm41,Humidity_pct_svm41,Raw_VOC_svm41,Raw_NOX_svm41\n");
    }
    rewind(logfile);

    // Accumulators for averaging
    float sht31_temp_sum = 0, sht31_hum_sum = 0;
    uint32_t sgp40_voc_sum = 0;
    int sample_counter = 0;

    /* Add feature autodetect i2c connections to channels of mux and
     * automatically read from all present, inserting them in csv correctly
     */

    while (1) {
        sensirion_i2c_hal_sleep_usec(1000000); // Sleep 1 second

    }
}