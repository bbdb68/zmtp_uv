#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

// type declaration
typedef struct input_stream_s input_stream_t;

// API
input_stream_t* input_stream_new(const size_t capacity);
void input_stream_delete(input_stream_t* stream);
int input_stream_append(input_stream_t* stream, char* data, size_t size);
size_t input_stream_pop(input_stream_t* stream, size_t size);

struct input_stream_s
{
  char* data;
  size_t size;
  size_t capacity;
};


#ifdef __cplusplus
}
#endif