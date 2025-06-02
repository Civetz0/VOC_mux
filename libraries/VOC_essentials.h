//
// Created by save0 on 02/06/2025.
//

#ifndef VOC_ESSENTIALS_H
#define VOC_ESSENTIALS_H
#include <stdint.h>


#endif //VOC_ESSENTIALS_H


#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#define CONFIG_FILE "../config.txt"


/**
 * mux_port_select() - This command selects one of the eight port of the multiplexer for i2c communication
 *
 * @param mux_addr I2C address of the multiplexer
 *
 * @param mux_port Port of the multiplexer to select (from 0 to 7)
 *
 * @return 0 on success, an error code otherwise
 */
int16_t mux_port_select(uint8_t mux_addr, uint8_t mux_port);

/**
 * mux_i2c_detect() - This command scans the i2c addresses range and returns 0 if it detects any device other
 * than the multiplexer
 *
 * @param mux_addr I2C address of the multiplexer
 *
 * @return 0 on success, 1 otherwise
 */
int16_t mux_i2c_detect (uint8_t mux_addr);

/**
 * get_timestamp() - This command saves the current time in a string buffer
 *
 * @param buffer String buffer where to save the time stamp
 *
 * @param size Size of buffer
 */
void get_timestamp(char* buffer, size_t size);

/**
 * read_config() - This function reads from a config file the temperature and humidity offset
 *
 * @param oversample_count Pointer to the oversampling multiplier variable to be updated
 *
 * @param humidity_offset Pointer to the humidity offset in percentage variable to be updated
 *
 * @return 0 on success, -1 if file is not found
 */
int read_config(int* oversample_count, float* humidity_offset);