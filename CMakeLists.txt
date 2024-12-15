# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(serial)

##### Remote Control #####
target_sources(app PRIVATE Proximity_alert/src/main.cpp Proximity_alert/src/engine.cpp Proximity_alert/src/nrf24.cpp Proximity_alert/src/hc-sr04.cpp)
target_include_directories(app PRIVATE Proximity_alert/include)