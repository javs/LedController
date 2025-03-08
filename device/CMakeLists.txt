cmake_minimum_required(VERSION 3.19)
cmake_policy(VERSION 3.19...3.22)


set(MBED_APP_JSON_PATH mbed_app.json5)
include(mbed-os/tools/cmake/mbed_toolchain_setup.cmake)

project(leds)
include(mbed_project_setup)

add_subdirectory(mbed-os)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/led_component.cpp
    src/led_controller.cpp
    src/settings.cpp
    src/usb_led_device.cpp
    src/apds9960_driver.cpp
    src/led_curve.cpp
    src/auto_led_controller.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        mbed-os
        mbed-storage-tdbstore
        mbed-usb
)

mbed_set_post_build(${PROJECT_NAME})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
