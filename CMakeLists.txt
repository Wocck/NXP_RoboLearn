# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(serial)

if(NOT DEFINED SELECTED_PROJECT)
    message(FATAL_ERROR "Please specify a project to build with -DSELECTED_PROJECT=<PROJECT_NAME>")
endif()

if(SELECTED_PROJECT STREQUAL "I2C_OLED")
    target_sources(app PRIVATE Projects/I2C_OLED/src/main.cpp Projects/I2C_OLED/src/oled.cpp)
    target_include_directories(app PRIVATE Projects/I2C_OLED/include)

elseif(SELECTED_PROJECT STREQUAL "ST1140")
    target_sources(app PRIVATE Projects/ST1140/main.cpp)

elseif(SELECTED_PROJECT STREQUAL "HCSR04")
    target_sources(app PRIVATE Projects/HCSR04/main.cpp)

elseif(SELECTED_PROJECT STREQUAL "SPI_NRF24L01")
    target_sources(app PRIVATE Projects/SPI_NRF24L01/src/main.cpp Projects/SPI_NRF24L01/src/nrf24.cpp)
    target_include_directories(app PRIVATE Projects/SPI_NRF24L01/include)

elseif(SELECTED_PROJECT STREQUAL "UART_BRIDGE")
    target_sources(app PRIVATE Projects/UART_BRIDGE/src/main.cpp Projects/UART_BRIDGE/src/uart_bridge.cpp)
    target_include_directories(app PRIVATE Projects/UART_BRIDGE/include)

else()
    message(FATAL_ERROR "Unknown project: ${SELECTED_PROJECT}")
endif()
