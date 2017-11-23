

set(LIBUVDIR ../uv/libuv)
include_directories(${LIBUVDIR}/include)


set(SRC main.cpp idle.cpp)

add_executable(tests ${SRC})
target_link_libraries(tests uv)
