
set(LIBUVDIR ../uv/libuv)
include_directories(${LIBUVDIR}/include)

set(SRC zmtp.h zmtp.c
        input_stream.h input_stream.c
	zmtp_greetings.h zmtp_greetings.c
	)

set(LIBNAME zmtp_uv)
add_library(${LIBNAME} ${SRC})
target_link_libraries(${LIBNAME} uv)
