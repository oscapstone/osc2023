set(CMAKE_LINKER ${LLVM_PATH}/ld.lld)

set(LINKER_SCRIPT "linker.ld")

set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib")
set(CMAKE_SHARED_LINKER_FLAGS "-nostdlib")

file(GLOB BOOTLOADER_SOURCES "*.c" "*.S")

add_executable(bootloader.elf ${BOOTLOADER_SOURCES})

target_link_libraries(bootloader.elf mylib)
target_include_directories(bootloader.elf PRIVATE ./)

set_target_properties(bootloader.elf PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT}")
