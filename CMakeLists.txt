cmake_minimum_required(VERSION 3.0.2)
project(growstack C ASM)

set(CMAKE_C_STANDARD 11)

SET(GCC_CUSTOM_BOTH_FLAGS "-Wall --save-temps")
SET(GCC_CUSTOM_COMPILE_FLAGS "-fno-pic -fno-pie -Wa,-msyntax=intel -masm=intel")
SET(GCC_CUSTOM_LINK_FLAGS "-no-pie")

SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_CUSTOM_BOTH_FLAGS} ${GCC_CUSTOM_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_CUSTOM_BOTH_FLAGS} ${GCC_CUSTOM_LINK_FLAGS}")

set(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")

add_executable(growstack main.c)
