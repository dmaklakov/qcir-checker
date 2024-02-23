cmake_minimum_required(VERSION 3.12)
project(qcir-checker)

# Define the sources.
set(SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/Scanner.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ParserLight.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/qcir.cpp
)

# Actually define the executable.
add_executable(qcir.exe ${SRCS})

# Enable Link-Time-Optimization (LTO) if supported.
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
  message(STATUS "IPO / LTO supported")
  if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "IPO / LTO enabled")
    set_property(TARGET qcir.exe PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
else()
  message(STATUS "IPO / LTO not supported: <${error}>")
endif()

# Add more warnings to the compiler output.
target_compile_options(qcir.exe PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
