file(GLOB LIB_SOURCES "*.c")
add_library(mylib STATIC ${LIB_SOURCES})
target_include_directories(mylib PUBLIC ${CMAKE_SOURCE_DIR}/include)
