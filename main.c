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
#include "libraries/VOC_essentials.h"

#define LOG_DIR "../logs"

int main(int argc, char* argv[]) {
    char filename[128];
    char timestamp[32];

    // Generate timestamp for filename
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", t);

    if (argc >= 2) {
        snprintf(filename, sizeof(filename), "%s/%s_%s.csv", LOG_DIR, argv[1], timestamp);
    } else {
        snprintf(filename, sizeof(filename), "%s/log_%s.csv", LOG_DIR, timestamp);
    }

    int oversample_count = 5;
    float sht31_humidity_offset = 0;

    if (read_config(&oversample_count, &sht31_humidity_offset) != 0) {
        printf("Using default config: oversample_count = %d, humidity_offset = %.2f\n", oversample_count, sht31_humidity_offset);
    } else {
        printf("Loaded config: oversample_count = %d, humidity_offset = %.2f\n", oversample_count, sht31_humidity_offset);
    }

    mkdir(LOG_DIR, 0755);

    FILE* logfile = fopen(filename, "a");
    if (!logfile) {
        perror("Failed to open log file");
        return 1;
    }

    // Write CSV header if file is empty
    fseek(logfile, 0, SEEK_END);
    if (ftell(logfile) == 0) {
        fprintf(logfile, "Timestamp");
        for (int i = 0; i < MAX_PORTS; i++) {
            fprintf(logfile, ",T%d,H%d,VOC%d", i, i, i);
        }
        fprintf(logfile, "\n");
    }

    sensirion_i2c_hal_init();
    sht3x_init(SHT31_I2C_ADDR_44);
    mux_init_address(TCA_ADDR_70);

    SensorAccumulator accum[MAX_PORTS];
    reset_accumulators(accum);

    while (1) {
        for (int i = 0; i < oversample_count; i++) {
            sample_all_ports(accum, sht31_humidity_offset);
            sensirion_i2c_hal_sleep_usec(1000000); // 1 second delay per sample
        }

        get_timestamp(timestamp, sizeof(timestamp));
        finalize_averages(logfile, accum, oversample_count, timestamp);
        reset_accumulators(accum);
    }

    fclose(logfile);
    return 0;
}
