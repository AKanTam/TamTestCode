cmake_minimum_required (VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (demo  LANGUAGES C CXX)

if (WIN32)
add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
endif()

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if (NOT MSVC)
endif()

add_executable (main)

target_include_directories(main PUBLIC include)

aux_source_directory (src SRC_LIST)

target_sources (main PUBLIC ${SRC_LIST})