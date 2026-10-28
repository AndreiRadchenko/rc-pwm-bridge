idf_component_register(SRCS "startup_policy.cpp" "gpio_safe_output.cpp" "monotonic_clock.cpp" "local_diagnostics.cpp"
    INCLUDE_DIRS "include" REQUIRES bridge_core esp_driver_gpio esp_driver_usb_serial_jtag esp_timer esp_hw_support spi_flash freertos)
get_filename_component(BOARD_PROFILE_DIR "${BOARD_PROFILE_HEADER}" DIRECTORY)
target_include_directories(${COMPONENT_LIB} PRIVATE "${BOARD_PROFILE_DIR}")
set_target_properties(${COMPONENT_LIB} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=c++17)
