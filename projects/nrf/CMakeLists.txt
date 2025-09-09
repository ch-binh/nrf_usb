cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf)

# include common USB cmake from Zephyr
include(${ZEPHYR_BASE}/samples/subsys/usb/common/common.cmake)

# Collect all sources in subdirectories
file(GLOB app_sources
    src/*.c
    src/app/protocol/at/*.c
    src/app/sys/*.c
    src/hw/hal/*.c
)

target_sources(app PRIVATE ${app_sources})

# Add include directories
target_include_directories(app PRIVATE
    src
    src/app/protocol/at
    src/app/sys
    src/hw/hal
)