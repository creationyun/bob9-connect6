# CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

set (PROJECT_VERSION "0.1")

project (connect6_protocol VERSION ${PROJECT_VERSION})

set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(connect6_protocol SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/connect6_protocol.cpp
)

set_target_properties(connect6_protocol PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    # G++
    target_compile_options(connect6_protocol PRIVATE -Wall -Wextra)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    # MSVC
    target_compile_options(connect6_protocol PRIVATE /EHsc /MTd /W2 /c)
    target_compile_definitions(connect6_protocol PRIVATE WIN_EXPORT)
endif()
