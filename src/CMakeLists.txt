
set(LIBUVDIR ../uv/libuv)
include_directories(${LIBUVDIR}/include)

add_library(zmtp_uv zmtp.h zmtp.c input_stream.h input_stream.c)
target_link_libraries(zmtp_uv uv)
