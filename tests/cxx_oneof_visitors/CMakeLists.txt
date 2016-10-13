cmake_minimum_required(VERSION 3.5)
project(cxx_oneof_visitors)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../extra)

find_package(Nanopb REQUIRED MODULE)

nanopb_generate_cpp(pb_sources pb_headers oneof_visitors.proto)
add_executable(cxx_oneof_visitors main.cpp ${pb_sources} ${pb_headers})
set_target_properties(cxx_oneof_visitors
    PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
)
target_include_directories(cxx_oneof_visitors
    PRIVATE
        ${NANOPB_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}
)
