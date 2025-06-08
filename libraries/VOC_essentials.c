#include "VOC_essentials.h"
#include "sensirion_i2c_hal.h"
#include <stdio.h>
#include <time.h>

static uint8_t _mux_addr;

void mux_init_address(uint8_t mux_addr) {
    _mux_addr = mux_addr;
}

int16_t mux_port_select(uint8_t mux_port) {
    if (mux_port > 7) return 1;
    uint8_t data = 1 << mux_port;
    return sensirion_i2c_hal_write(_mux_addr, &data, 1);
}

int16_t mux_i2c_detect() {
    for (uint8_t addr = 0; addr <= 127; addr++) {
        if (addr == _mux_addr) continue;
        if (sensirion_i2c_hal_write(addr, NULL, 0) == 0) {
            return 0; // Device detected
        }
    }
    return 1; // No devices found
}

void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    strftime(buffer, size, "%Y-%m-%dT%H:%M:%S", localtime(&now));
}

int read_config(int* oversample_count, float* humidity_offset) {
    FILE* config_file = fopen(CONFIG_FILE, "r");
    if (!config_file) {
        fprintf(stderr, "Config file not found. Using defaults.\n");
        return -1;
    }

    char line[128];
    while (fgets(line, sizeof(line), config_file)) {
        char key[64], value[64];
        if (line[0] == '\0' || line[0] == '#') continue;
        if (sscanf(line, "%63s = %63s", key, value) == 2) {
            if (strcmp(key, "oversample_count") == 0) {
                *oversample_count = atoi(value);
                if (*oversample_count <= 0) *oversample_count = 5;
            } else if (strcmp(key, "humidity_offset") == 0) {
                *humidity_offset = atof(value);
                if (*humidity_offset < 0) *humidity_offset = 0;
            }
        }
    }

    fclose(config_file);
    return 0;
}

int16_t measure_oversampled(int oversample_count, float humidity_offset, float* avg_temp, float* avg_hum, uint16_t* avg_raw_voc) {
    float temp_sum = 0, hum_sum = 0;
    unsigned int voc_sum = 0;
    int16_t error = 0;

    for (int i = 0; i < oversample_count; i++) {
        float t = 0, h = 0;
        uint16_t voc = 0;
        error = single_measure(&h, &t, &voc, humidity_offset);
        sensirion_i2c_hal_sleep_usec(1000000);
        if (error) return error;

        temp_sum += t;
        hum_sum += h;
        voc_sum += voc;
    }

    *avg_temp = temp_sum / oversample_count;
    *avg_hum = hum_sum / oversample_count;
    *avg_raw_voc = voc_sum / oversample_count;

    return 0;
}

int16_t single_measure(float* humidity, float* temperature, uint16_t* raw_voc, float humidity_offset) {
    uint16_t h_ticks = 0, t_ticks = 0;
    int16_t error = sht3x_measure_single_shot(REPEATABILITY_HIGH, false, &t_ticks, &h_ticks);
    if (error != NO_ERROR) return error;

    h_ticks += (uint16_t)((humidity_offset * 65535.0f) / 100.0f);

    *humidity = signal_humidity(h_ticks);
    *temperature = signal_temperature(t_ticks);

    error = sgp40_measure_raw_signal(h_ticks, t_ticks, raw_voc);
    return error;
}

void sample_all_ports(SensorAccumulator accum[], float humidity_offset) {
    for (int port = 0; port < MAX_PORTS; port++) {
        mux_port_select(port);

        if (!mux_i2c_detect()) {
            float t = 0, h = 0;
            uint16_t voc = 0;
            if (single_measure(&h, &t, &voc, humidity_offset) == 0) {
                accum[port].temp_sum += t;
                accum[port].hum_sum += h;
                accum[port].voc_sum += voc;
                accum[port].sample_count++;

                printf("Port %d | Temp: %.2f °C | Humidity: %.2f %% | VOC: %u ticks\n", port, t, h, voc);
            }
        }
    }
}

void finalize_averages(FILE* logfile, SensorAccumulator accum[], int oversample_count, const char* timestamp) {
    char csv_row[1024] = "";
    snprintf(csv_row, sizeof(csv_row), "%s", timestamp);

    for (int port = 0; port < MAX_PORTS; port++) {
        if (accum[port].sample_count == oversample_count) {
            float avg_temp = accum[port].temp_sum / oversample_count;
            float avg_hum = accum[port].hum_sum / oversample_count;
            uint16_t avg_voc = accum[port].voc_sum / oversample_count;

            snprintf(csv_row + strlen(csv_row), sizeof(csv_row) - strlen(csv_row),
                     ",%.2f,%.2f,%u", avg_temp, avg_hum, avg_voc);
        } else {
            snprintf(csv_row + strlen(csv_row), sizeof(csv_row) - strlen(csv_row), ",NaN,NaN,NaN");
        }
    }

    fprintf(logfile, "%s\n", csv_row);
    fflush(logfile);
}

void reset_accumulators(SensorAccumulator accum[]) {
    for (int i = 0; i < MAX_PORTS; i++) {
        accum[i] = (SensorAccumulator){0};
    }
}
