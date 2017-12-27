#include "catch.hpp"

#include "input_stream.h"

TEST_CASE("input_stream")
{
  size_t initial_cap = 5;
  input_stream_t * is = input_stream_new(initial_cap);

  input_stream_append(is, "0123",4);
  CHECK(input_stream_capacity(is) == initial_cap);
  CHECK(input_stream_size(is) == 4);

  input_stream_append(is, "5678", 4);
  CHECK(input_stream_capacity(is) == 8);
  CHECK(input_stream_size(is) == 8);

  input_stream_pop(is, 3);
  CHECK(input_stream_capacity(is) == 8);
  CHECK(input_stream_size(is) == 5);
  char* data = input_stream_data(is);
  CHECK(data[0] == '3');
  CHECK(data[1] == '5');
  CHECK(data[2] == '6');
  CHECK(data[3] == '7');
  CHECK(data[4] == '8');

  input_stream_delete(is);
}