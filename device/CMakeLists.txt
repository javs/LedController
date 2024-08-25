cmake_minimum_required(VERSION 3.19.0)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET device)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "")

include(${MBED_PATH}/tools/cmake/app.cmake)

project(${APP_TARGET})

add_subdirectory(${MBED_PATH})

# workaround mbed not catching up 
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-register>)

add_executable(${APP_TARGET}
    src/main.cpp
    src/led_component.cpp
    src/led_controller.cpp
    src/settings.cpp
    src/usb_led_device.cpp
    src/apds9960_driver.cpp
    src/led_curve.cpp
    src/temp_manager.cpp
)

target_include_directories(${APP_TARGET}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/
)

target_link_libraries(${APP_TARGET}
    PRIVATE
        mbed-os
        mbed-events
        mbed-storage-tdbstore
        mbed-usb
)

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
