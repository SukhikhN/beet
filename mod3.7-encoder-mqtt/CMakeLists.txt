cmake_minimum_required(VERSION 3.16.0)
set(SDKCONFIG_DEFAULTS "sdkconfig.defaults;sdkconfig.local")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(mod3.7-encoder-mqtt)
