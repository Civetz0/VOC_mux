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

    fseek(logfile, 0, SEEK_END);
    if (ftell(logfile) == 0) {
        fprintf(logfile, "Timestamp");
        for (int i = 0; i < MAX_PORTS; i++) {
            fprintf(logfile, ",T%d,H%d,VOC%d", i + 1, i + 1, i + 1);
        }
        fprintf(logfile, "\n");
    }

    sensirion_i2c_hal_init();
    sht3x_init(SHT31_I2C_ADDR_44);
    mux_init_address(TCA_ADDR_70);

    float temperature_c = 0;
    float humidity_p = 0;
    uint16_t raw_voc = 0;

    while (1) {
        char csv_row[1024] = "";
        get_timestamp(timestamp, sizeof(timestamp));
        snprintf(csv_row + strlen(csv_row), sizeof(csv_row) - strlen(csv_row), "%s", timestamp);

        for (int port = 0; port < MAX_PORTS; port++) {
            mux_port_select(port);

            if (!mux_i2c_detect()) {
                if (measure_oversampled(oversample_count, sht31_humidity_offset, &temperature_c, &humidity_p, &raw_voc) == 0) {
                    snprintf(csv_row + strlen(csv_row), sizeof(csv_row) - strlen(csv_row),
                             ",%.2f,%.2f,%u", temperature_c, humidity_p, raw_voc);
                } else {
                    fprintf(stderr, "Measurement failed on port %d\n", port);
                    snprintf(csv_row + strlen(csv_row), sizeof(csv_row) - strlen(csv_row),
                             ",NaN,NaN,NaN");
                }
            } else {
                snprintf(csv_row + strlen(csv_row), sizeof(csv_row) - strlen(csv_row),
                         ",NaN,NaN,NaN");
            }
        }

        fprintf(logfile, "%s\n", csv_row);
        fflush(logfile);
        sensirion_i2c_hal_sleep_usec(1000000);
    }

    fclose(logfile);
    return 0;
}
