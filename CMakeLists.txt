# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Timers_pwm)

if(NOT DEFINED SELECTED_PROJECT)
    message(FATAL_ERROR "Please specify a project to build with -DSELECTED_PROJECT=<PROJECT_NAME>")
endif()


if(SELECTED_PROJECT STREQUAL "LED_PWM")
    target_sources(app PRIVATE Projects/LED_pwm/main.cpp)

elseif(SELECTED_PROJECT STREQUAL "LED_HARDWARE_TIMER")
    target_sources(app PRIVATE Projects/LED_hardware_timer/main.cpp)

elseif(SELECTED_PROJECT STREQUAL "LED_PROGRAM_TIMER")
    target_sources(app PRIVATE Projects/LED_program_timer/main.cpp)

elseif(SELECTED_PROJECT STREQUAL "DC_MOTOR")
    target_sources(app PRIVATE Projects/DC_motor/src/main.cpp Projects/DC_motor/src/engine.cpp)
    target_include_directories(app PRIVATE Projects/DC_motor/include)

else()
    message(FATAL_ERROR "Unknown project: ${SELECTED_PROJECT}")
endif()
