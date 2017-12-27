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

// accessors
size_t input_stream_size(input_stream_t* stream);
size_t input_stream_capacity(input_stream_t* stream);
char* input_stream_data(input_stream_t* stream);

#ifdef __cplusplus
}
#endif