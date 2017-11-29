

set(LIBUVDIR ../../uv/libuv)
include_directories(${LIBUVDIR}/include ../../src)


set(SRC main_client.c)

add_executable(client ${SRC})
target_link_libraries(client uv zmtp_uv)
