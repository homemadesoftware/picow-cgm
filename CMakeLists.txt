cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(cgm)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(hello_world
main.c
EPD_2in13_V3.c
picow_tcp_client.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(hello_world pico_stdlib)


target_compile_definitions(hello_world PRIVATE
        )

target_include_directories(hello_world PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )

add_subdirectory(lib/Config)
add_subdirectory(lib/e-Paper)
add_subdirectory(lib/Fonts)
add_subdirectory(lib/GUI)
#add_subdirectory(examples)


include_directories(lib/Config)
include_directories(lib/GUI)


target_link_libraries(hello_world
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        GUI 
        Fonts 
        Config 
        pico_stdlib 
        hardware_spi
        )

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hello_world)


