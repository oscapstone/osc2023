set(CMAKE_LINKER ${LLVM_PATH}/ld.lld)
set(LINKER_SCRIPT "linker.ld")

set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib")
set(CMAKE_SHARED_LINKER_FLAGS "-nostdlib")

file(GLOB KERNEL_SOURCES "*.c" "*.S")

add_executable(kernel.elf ${KERNEL_SOURCES})

target_link_libraries(kernel.elf mylib)
target_include_directories(kernel.elf PRIVATE ./)

set_target_properties(kernel.elf PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT}")
