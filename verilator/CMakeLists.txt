cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0074 NEW)

project(FionaSim)

set(CMAKE_C_STANDARD 11)

find_package(Python 3.6 REQUIRED Development)
find_package(verilator HINTS $ENV{VERILATOR_ROOT})

if (NOT verilator_FOUND)
  message(FATAL_ERROR "Verilator was not found. Either install it, or set the VERILATOR_ROOT environment variable")
endif()

include_directories(${Python_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR} mod)

add_executable(${PROJECT_NAME}
  main.cpp
  mod/bridge.cc
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${Python_LIBRARIES})

file(COPY "${PROJECT_SOURCE_DIR}/pyfunc/" DESTINATION "${PROJECT_BINARY_DIR}/pyfunc/")

set(VERILATOR_TOP_MODULE)

verilate(${PROJECT_NAME}
  INCLUDE_DIRS "."
  SOURCES top.sv
)

