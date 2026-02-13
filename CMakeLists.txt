cmake_minimum_required(VERSION 3.13)

set(PICO_BTSTACK_PATH ${CMAKE_CURRENT_LIST_DIR}/btstack)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")

include(pico_sdk_import.cmake)

project(my_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(main_app
    source/main_app.c
    source/bt_hid.c
    source/peripheral_setup.c
    source/display_functions.c
    source/display_font.c
)

pico_enable_stdio_usb(main_app 1)
pico_enable_stdio_uart(main_app 0)


target_include_directories(main_app PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/includes
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main_app    
    pico_stdlib
    pico_cyw43_arch_none
    pico_btstack_classic
    pico_btstack_ble
    pico_btstack_cyw43
    pico_multicore
    hardware_i2c
    hardware_spi
    hardware_gpio
)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main_app)