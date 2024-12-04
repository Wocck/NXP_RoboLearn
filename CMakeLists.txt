# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(serial)

##### LED pwm #####
#target_sources(app PRIVATE Projects/LED_pwm/main.cpp)

##### DC motor #####
#target_sources(app PRIVATE Projects/DC_motor/src/main.cpp Projects/DC_motor/src/aht40.cpp)
#target_include_directories(app PRIVATE Projects/DC_motor/include)