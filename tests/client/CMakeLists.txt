

set(LIBUVDIR ../../uv/libuv)
include_directories(${LIBUVDIR}/include)


set(SRC main_client.c)

add_executable(client ${SRC})
target_link_libraries(client uv)
