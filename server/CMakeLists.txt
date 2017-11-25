

set(LIBUVDIR ../uv/libuv)
include_directories(${LIBUVDIR}/include)


set(SRC main_server.c)

add_executable(server ${SRC})
target_link_libraries(server uv)
