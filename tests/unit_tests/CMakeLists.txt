

set(LIBUVDIR ../../uv/libuv)
include_directories(${LIBUVDIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)


set(SRC main.cpp idle.cpp test_input_stream.cpp)

add_executable(tests ${SRC})
target_link_libraries(tests uv zmtp_uv)
