# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(serial)

##### AHT40 #####
target_sources(app PRIVATE AHT40/src/main_aht40.cpp AHT40/src/aht40.cpp)
target_include_directories(app PRIVATE AHT40/include)

##### ST1140 #####
#target_sources(app PRIVATE ST1140/main_st1140.cpp)

##### CSR04 #####H
#target_sources(app PRIVATE HCSR04/main_hcsr04.cpp)