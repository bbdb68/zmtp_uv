#pragma once

#include "uv.h"
#include "input_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

// type declaration
typedef struct zmtp_stream_s zmtp_stream_t;
typedef void(*zmtp_stream_read_cb)(zmtp_stream_t* stream, void *msg, int size);

// API
zmtp_stream_t* zmtp_stream_new(uv_stream_t* stream, zmtp_stream_read_cb read_cb);
void zmtp_stream_delete(zmtp_stream_t* stream);
int zmtp_stream_connect(zmtp_stream_t* stream, const char* address);  /* zmq address scheme tcp://<target_ip>:port */
int zmtp_stream_bind(zmtp_stream_t* stream, const char* address);
int zmtp_stream_send(zmtp_stream_t* stream, void* data, size_t size);

// TODO make -private- method
void zmtp_send_greetings(uv_stream_t* stream);

// todo private
enum zmtp_stream_state { greetings1, greetings2, frame};

// -------------------------------
// zmtp stream definition
// -------------------------------
struct zmtp_stream_s
{
  uv_stream_t* stream;              // the transport stream
  enum zmtp_stream_state status;
  input_stream_t* input_stream;     // input buffer
  zmtp_stream_read_cb read_cb;
};


#ifdef __cplusplus
}
#endif