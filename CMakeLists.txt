# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(interrupts)

##### IRQ #####
#target_sources(app PRIVATE ISR_Api/main.cpp)

##### NRF24_IRQ #####
target_sources(app PRIVATE NRF24_IRQ/src/main.cpp NRF24_IRQ/src/nrf24.cpp)
target_include_directories(app PRIVATE NRF24_IRQ/include)