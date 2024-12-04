# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(interrupts)

##### IRQ #####
#target_sources(app PRIVATE ISR_Api/main.cpp)

##### NRF24_example #####
# target_sources(app PRIVATE Projects/NRF24_example/src/main.cpp Projects/NRF24_example/src/nrf24.cpp)
# target_include_directories(app PRIVATE Projects/NRF24_example/include)

##### NRF24 #####
target_sources(app PRIVATE Projects/NRF24/src/main.cpp Projects/NRF24/src/nrf24.cpp)
target_include_directories(app PRIVATE Projects/NRF24/include)