#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zmtp.h"

void echo_on_read(zmtp_stream_t* stream, void *msg, size_t size)
{
  char* content = malloc((size + 1) * sizeof(char));
  memcpy(content, msg, size);
  content[size] = 0x00;
  printf("msg='%s'\n", content);
  zmtp_stream_send(stream, msg, size);
  free(content);
  uv_stop(stream->stream->loop);
}

int main() {
  printf("Hello, I am the server\n");

  uv_loop_t* loop = uv_default_loop();

  uv_tcp_t* server = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, server);  


  zmtp_stream_t* s = zmtp_stream_new((uv_stream_t*)server, echo_on_read);
  zmtp_stream_bind(s, "<not yet parsed address>");

  return uv_run(loop, UV_RUN_DEFAULT);
}
