

set(LIBUVDIR ../../uv/libuv)
include_directories(${LIBUVDIR}/include ../../src)

set(libraries uv zmtp_uv)
if(CMAKE_COMPILER_IS_GNUCXX)
  list(APPEND libraries pthread)
endif()

set(SRC main_server.c)

add_executable(server ${SRC})
target_link_libraries(server ${libraries})
