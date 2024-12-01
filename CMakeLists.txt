# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(serial)

##### AHT40 #####
#target_sources(app PRIVATE AHT40/src/main.cpp AHT40/src/aht40.cpp)
#target_include_directories(app PRIVATE AHT40/include)

##### ST1140 #####
#target_sources(app PRIVATE ST1140/main.cpp)

##### CSR04 #####
#target_sources(app PRIVATE HCSR04/main.cpp)

##### NRF24 #####
target_sources(app PRIVATE NRF24L01/src/main.cpp NRF24L01/src/nrf24.cpp)
target_include_directories(app PRIVATE NRF24L01/include)