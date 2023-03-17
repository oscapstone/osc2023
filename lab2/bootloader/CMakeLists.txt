set(CMAKE_LINKER ${LLVM_PATH}/ld.lld)

set(LINKER_SCRIPT "linker.ld")

set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib")
set(CMAKE_SHARED_LINKER_FLAGS "-nostdlib")

file(GLOB BOOTLOADER_SOURCES "*.c" "*.S")

add_executable(bootloader ${BOOTLOADER_SOURCES})

target_link_libraries(bootloader mylib)
target_include_directories(bootloader PRIVATE ./)

# set_target_properties(bootloader PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT})
set_target_properties(bootloader PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT}")
