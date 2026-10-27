# Korvo 爪爪 — ESP-IDF 工程根
# 需要 ESP-IDF >= 5.4，目标 esp32s31
# 在实板上添加 BSP 依赖后执行: idf.py set-target esp32s31 && idf.py build

cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(korvo_buddy)
