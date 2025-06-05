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
 * read_config() - This function reads from a config file the temperature and humidity offset
 *
 * @param oversample_count Pointer to the oversampling multiplier variable to be updated
 *
 * @param humidity_offset Pointer to the humidity offset in percentage variable to be updated
 *
 * @return 0 on success, -1 if file is not found
 */
int read_config(int* oversample_count, float* humidity_offset);



int16_t measure_oversampled(int oversample_count, float sht31_humidity_offset, float * avg_temp, float * avg_hum, uint16_t * avg_raw_voc );

int16_t single_measure(float * humidity, float * temperature, uint16_t * raw_voc, float humidity_offset);