add_library(mbed-nucleo-f303re-ledstrip INTERFACE)

target_sources(mbed-nucleo-f303re-ledstrip
    INTERFACE
        PeripheralPins.c
)

target_include_directories(mbed-nucleo-f303re-ledstrip
    INTERFACE
        .
)

target_link_libraries(mbed-nucleo-f303re-ledstrip INTERFACE mbed-stm32f303xe)
