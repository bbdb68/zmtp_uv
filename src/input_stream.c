#include "input_stream.h"

#include <stdlib.h>

// ------------------------------------------------------------
// allocate new input stream
// ------------------------------------------------------------
input_stream_t* input_stream_new(const size_t capacity)
{
  input_stream_t* result = malloc(sizeof(input_stream_t));
  if( result )
    result->data = malloc(capacity * sizeof(char));
  result->size = 0;
  result->capacity = capacity;
  return result;
  
}

// ------------------------------------------------------------
// free input stream
// ------------------------------------------------------------
void input_stream_delete(input_stream_t* stream)
{
  free(stream->data);
  free(stream);
}

// ------------------------------------------------------------
// append data to stream (with copy)
// ------------------------------------------------------------
int input_stream_append(input_stream_t* stream, char* data, size_t size)
{
  if (stream->size + size > stream->capacity) // reallocation required
  {
    stream->capacity = stream->size + size;
    stream->data = realloc(stream->data, stream->capacity);
    if (!stream->data)
      return -1;
  }
  memcpy(stream->data + stream->size, data, size);
  stream->size += size;
  return 0;
}

// ------------------------------------------------------------
// remove data at stream's head. data is just overwritten, 
// should have been used/copied before
// ------------------------------------------------------------
size_t input_stream_pop(input_stream_t* stream, size_t size)
{
  if (size > stream->size)
    size = stream->size;
  size_t moved_size = stream->size - size;
  memmove(stream->data, stream->data + size, moved_size);
  stream->size -= size;
  return size;
}
