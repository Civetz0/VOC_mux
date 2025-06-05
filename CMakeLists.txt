cmake_minimum_required(VERSION 3.31)
project(VOC_multiplexer C)

set(CMAKE_C_STANDARD 11)

# Add all C source files
set(SOURCES
        main.c
        libraries/VOC_essentials.c
        libraries/sensirion_i2c.c
        libraries/sensirion_i2c_hal.c
        libraries/sensirion_common.c
        libraries/sgp40_i2c.c
        libraries/sht3x_i2c.c
)

# Create the executable
add_executable(VOC_multiplexer ${SOURCES})

# Link with pthread and math if needed (common on Linux)
target_link_libraries(VOC_multiplexer
        pthread
        m
)
