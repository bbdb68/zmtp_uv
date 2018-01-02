

set(LIBUVDIR ../../uv/libuv)
include_directories(${LIBUVDIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

set(libraries uv zmtp_uv)
if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-std=gnu++11)
  list(APPEND libraries pthread)
endif()

set(SRC main.cpp idle.cpp test_input_stream.cpp tcp_echo.cpp)

add_executable(tests ${SRC})
target_link_libraries(tests ${libraries})
