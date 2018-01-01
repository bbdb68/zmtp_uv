

set(LIBUVDIR ../../uv/libuv)
include_directories(${LIBUVDIR}/include ../../src)

set(libraries uv zmtp_uv)
if(CMAKE_COMPILER_IS_GNUCXX)
  list(APPEND libraries pthread)
endif()

set(SRC main_client.c)

add_executable(client ${SRC})
target_link_libraries(client ${libraries})
