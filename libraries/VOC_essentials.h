//
// Created by save0 on 02/06/2025.
//

#ifndef VOC_ESSENTIALS_H
#define VOC_ESSENTIALS_H

#include <stdint.h>
#include "sensirion_common.h"
#include "sensirion_i2c.h"
#include "sensirion_i2c_hal.h"
#include "sgp40_i2c.h"
#include "sht3x_i2c.h"


#endif //VOC_ESSENTIALS_H


#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#define CONFIG_FILE "../config.txt"

#define MAX_PORTS 8


#define TCA_ADDR_70 0x70
#define TCA_ADDR_71 0x71
#define TCA_ADDR_72 0x72
#define TCA_ADDR_73 0x73
#define TCA_ADDR_74 0x74
#define TCA_ADDR_75 0x75
#define TCA_ADDR_76 0x76
#define TCA_ADDR_77 0x77

/**
 * @struct SensorAccumulator
 * @brief A structure to accumulate sensor readings over multiple samples for averaging.
 *
 * This structure is used during oversampling to store the running sum of sensor readings
 * and the number of valid samples collected for a given multiplexer port.
 */
typedef struct {
    float temp_sum;        /**< Sum of temperature readings (°C). */
    float hum_sum;         /**< Sum of humidity readings (%RH). */
    uint32_t voc_sum;      /**< Sum of raw VOC signal readings (ticks). */
    int sample_count;      /**< Number of valid samples accumulated. */
} SensorAccumulator;

/**
 *  mux_init_address() - This function select the  of the multiplexer to use in following functions
 *
 *  @param mux_addr is the multiplexer address in the format 0xXX
 *
 */
void mux_init_address(uint8_t mux_addr);

/**
 * mux_port_select() - This command selects one of the eight port of the multiplexer for i2c communication
 *
 * @param mux_port Port of the multiplexer to select (from 0 to 7)
 *
 * @return 0 on success, an error code otherwise
 */
int16_t mux_port_select(uint8_t mux_port);

/**
 * mux_i2c_detect() - This command scans the i2c addresses range and returns 0 if it detects any device other
 * than the multiplexer
 *
 * @return 0 on success, 1 otherwise
 */
int16_t mux_i2c_detect ();

/**
 * get_timestamp() - This command saves the current time in a string buffer
 *
 * @param buffer String buffer where to save the time stamp
 *
 * @param size Size of buffer
 */
void get_timestamp(char* buffer, size_t size);

/**
 * read_config() - Reads configuration values from a file.
 *
 * This function reads two parameters from a configuration file:
 *  - The oversampling count, which determines how many individual measurements are averaged.
 *  - The humidity offset, which is used to correct sensor readings.
 *
 * @param oversample_count Pointer to an integer where the read oversampling count will be stored.
 * @param humidity_offset Pointer to a float where the read humidity offset (in %) will be stored.
 *
 * @return 0 on success, -1 if the configuration file is not found or cannot be read.
 */
int read_config(int* oversample_count, float* humidity_offset);


/**
 *
 * @param oversample_count Oversampling factor so that T_sampling = oversample_count*1s
 * @param sht31_humidity_offset Offset for humidity measure
 * @param avg_temp
 * @param avg_hum
 * @param avg_raw_voc
 * @return
 */
int16_t measure_oversampled(int oversample_count, float sht31_humidity_offset, float * avg_temp, float * avg_hum, uint16_t * avg_raw_voc );

/**
 * single_measure() - Performs a single sensor measurement for temperature, humidity, and VOC.
 *
 * This function reads temperature and humidity from the SHT31, applies the humidity offset,
 * and then measures raw VOC signal using the SGP40, providing the corrected humidity and temperature.
 *
 * @param humidity Pointer to float where the corrected humidity (%RH) will be stored.
 * @param temperature Pointer to float where the measured temperature (°C) will be stored.
 * @param raw_voc Pointer to uint16_t where the measured raw VOC signal (ticks) will be stored.
 * @param humidity_offset Offset in %RH applied to humidity for VOC compensation.
 *
 * @return 0 on success, error code if measurement fails.
 */
int16_t single_measure(float* humidity, float* temperature, uint16_t* raw_voc, float humidity_offset);


/**
 * sample_all_ports() - Performs one measurement per active sensor port and stores results in accumulators.
 *
 * This function loops over all multiplexer ports, selects each sensor, performs a single measurement,
 * and accumulates the results (temperature, humidity, VOC) for later averaging.
 *
 * @param accum Array of SensorAccumulator structures used to collect and sum measurements for each port.
 * @param humidity_offset Offset in %RH applied to humidity readings.
 */
void sample_all_ports(SensorAccumulator accum[], float humidity_offset);


/**
 * finalize_averages() - Computes final averages and writes them to the logfile.
 *
 * This function calculates the average temperature, humidity, and VOC values for each port
 * using the accumulated sums and writes a formatted CSV line to the logfile.
 *
 * @param logfile File pointer to the CSV log file.
 * @param accum Array of SensorAccumulator structures containing summed data.
 * @param oversample_count Number of measurements accumulated (used for averaging).
 * @param timestamp Current timestamp string to prefix the CSV line.
 */
void finalize_averages(FILE* logfile, SensorAccumulator accum[], int oversample_count, const char* timestamp);


/**
 * reset_accumulators() - Resets the measurement accumulators for all ports.
 *
 * This function sets the sum and count fields of each SensorAccumulator to zero,
 * preparing them for a new round of oversampling.
 *
 * @param accum Array of SensorAccumulator structures to reset.
 */
void reset_accumulators(SensorAccumulator accum[]);


