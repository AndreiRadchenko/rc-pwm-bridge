idf_component_register(SRCS "startup.cpp" INCLUDE_DIRS "include")
set_target_properties(${COMPONENT_LIB} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
target_compile_options(${COMPONENT_LIB} PRIVATE -std=c++11 -fno-rtti -fno-exceptions)
