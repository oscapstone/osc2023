cmake_minimum_required(VERSION 3.18.4)
project(osc)

enable_language(C ASM)

set(LLVM_PATH "/Users/ianlee/Code/llvm-related/llvm-install/bin")

set(CMAKE_C_COMPILER ${LLVM_PATH}/clang)
set(CMAKE_LINKER ${LLVM_PATH}/ld.lld)
set(CMAKE_ASM_COMPILER ${LLVM_PATH}/clang)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-a53+nosimd --target=aarch64-rpi3-elf -Wall -O2 -ffreestanding -nostdinc")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(bootloader)
add_subdirectory(lib)
add_subdirectory(kernel)
