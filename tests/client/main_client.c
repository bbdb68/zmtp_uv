#include <stdlib.h>

#include "uv.h"
#include "zmtp.h"
#include "input_stream.h"

void on_connect(zmtp_stream_connect_t* req, int status)
{
  if (status < 0)
  {
    printf("zmtp connection error %d\n.", status);
    exit(1);
  }
  char* data = malloc(6*sizeof(char)); // cannot send static data for now
  strcpy(data, "Hello");
  zmtp_stream_send(req->stream, data, 5); // send callback missing
}

void on_read(zmtp_stream_t* stream, void *msg, size_t size)
{
  printf("client on read %d bytes\n", size);
  char* response = malloc((size + 1) * sizeof(char));
  memcpy(response, msg, size);
  response[size] = 0x00;
  printf("reception = '%s'\n", response);
  free(response);
  uv_stop(stream->stream->loop);
}


// -------------------------
// main client
// -------------------------
int main()
{
  printf("Hello, I am the client\n");
  uv_loop_t* loop = uv_default_loop();

  uv_tcp_t* socket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, socket);
  //uv_tcp_keepalive(socket, 1, 6000);

  zmtp_stream_t* zmtp_stream = zmtp_stream_new((uv_stream_t*)socket, on_read);
  zmtp_stream_connect_t* connect_req = (zmtp_stream_connect_t*)malloc(sizeof(zmtp_stream_connect_t));
  int res = zmtp_stream_connect(connect_req, zmtp_stream, "<not yet parsed address>",on_connect);

  uv_run(loop, UV_RUN_DEFAULT);

  zmtp_stream_delete(zmtp_stream);
  free(socket);

  return 0;
}
