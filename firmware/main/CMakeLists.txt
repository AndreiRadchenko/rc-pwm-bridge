idf_component_register(SRCS "app_main.cpp" REQUIRES bridge_core platform_esp32)
set_target_properties(${COMPONENT_LIB} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
get_filename_component(BOARD_PROFILE_DIR "${BOARD_PROFILE_HEADER}" DIRECTORY)
target_include_directories(${COMPONENT_LIB} PRIVATE "${BOARD_PROFILE_DIR}")

target_compile_options(${COMPONENT_LIB} PRIVATE -std=c++17)
