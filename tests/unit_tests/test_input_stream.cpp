#include "catch.hpp"

#include "input_stream.h"

TEST_CASE("input_stream")
{
  size_t initial_cap = 5;
  input_stream_t * is = input_stream_new(initial_cap);

  input_stream_append(is, "0123",4);
  CHECK(is->capacity == initial_cap);
  CHECK(is->size == 4);

  input_stream_append(is, "5678", 4);
  CHECK(is->capacity == 8);
  CHECK(is->size == 8);

  input_stream_pop(is, 3);
  CHECK(is->capacity == 8);
  CHECK(is->size == 5);
  CHECK(is->data[0] == '3');
  CHECK(is->data[1] == '5');
  CHECK(is->data[2] == '6');
  CHECK(is->data[3] == '7');
  CHECK(is->data[4] == '8');

  input_stream_delete(is);
}