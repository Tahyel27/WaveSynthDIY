add_executable(${NAME} main.cpp ButtonArray.cpp AnalogArray.cpp)

pico_generate_pio_header(${NAME} ${CMAKE_CURRENT_LIST_DIR}/../asm/audio_device.pio)
pico_generate_pio_header(${NAME} ${CMAKE_CURRENT_LIST_DIR}/../asm/button_array.pio)

pico_set_program_name(${NAME} "firmware")
pico_set_program_version(${NAME} "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${NAME} 0)
pico_enable_stdio_usb(${NAME} 1)

# Add the standard library to the build
target_link_libraries(${NAME}
        pico_stdlib
        hardware_pio
        hardware_dma
        hardware_adc
)

set_target_properties(${NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Add the standard include files to the build
target_include_directories(${NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../include
)

pico_add_extra_outputs(${NAME})