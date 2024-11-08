# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(serial)

# Dodanie głównego pliku main_st1140.cpp oraz plików biblioteki aht40.cpp
target_sources(app PRIVATE src/main_aht40.cpp src/aht40.cpp)

# Ustawienie ścieżki do plików nagłówkowych
target_include_directories(app PRIVATE include)