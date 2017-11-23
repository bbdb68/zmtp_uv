#include "uv.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"


TEST_CASE("main")
{
  CHECK(1 == 1);
}

int counter = 0;
void wait_for_a_while(uv_idle_t* handle) {
  counter++;

  if (counter >= 1e5)
    uv_idle_stop(handle);
}


TEST_CASE("uv_idle")
{
  uv_idle_t idler;

  uv_idle_init(uv_default_loop(), &idler);
  uv_idle_start(&idler, wait_for_a_while);

  INFO("Idling...\n");
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  uv_loop_close(uv_default_loop());
}