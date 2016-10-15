cmake_minimum_required(VERSION 3.5)
project(cxx-test)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../extra)

find_package(Nanopb REQUIRED MODULE)

nanopb_generate_cpp(pb_sources pb_headers oneof-test.proto)
add_executable(cxx-test main.cpp oneof-test.cpp ${pb_sources} ${pb_headers})
set_target_properties(cxx-test
    PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
)
target_include_directories(cxx-test
    PRIVATE
        ${NANOPB_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}
)
