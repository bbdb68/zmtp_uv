#include "catch.hpp"

#include "uv.h"
#include "zmtp.h"
#include "input_stream.h"

static void server_echo_on_read(zmtp_stream_t* stream, void *msg, size_t size)
{
  char* content = (char*)malloc((size + 1) * sizeof(char));
  memcpy(content, msg, size);
  content[size] = 0x00;
  //printf("msg='%s'\n", content);
  CHECK(strcmp(content, "Hello") == 0);
  zmtp_stream_send(stream, content, size);
}

static void client_on_connect(zmtp_stream_connect_t* req, int status)
{
  if (status < 0)
  {
    printf("zmtp connection error %d\n.", status);
    exit(1);
  }
  char* data = (char*)malloc(6 * sizeof(char)); // cannot send static data for now
  strcpy_s(data, 6, "Hello");
  zmtp_stream_send(req->stream, data, 5); // send callback missing
}

static void client_on_read(zmtp_stream_t* stream, void *msg, size_t size)
{
  //printf("client on read %zd bytes\n", size);
  char* response = (char*)malloc((size + 1) * sizeof(char));
  memcpy(response, msg, size);
  response[size] = 0x00;
  CHECK(strcmp( response,"Hello")==0 );

  //printf("reception = '%s'\n", response);
  free(response);
  uv_stop(stream->stream->loop);
}


// -------------------------
// main client
// -------------------------
TEST_CASE("tcp_echo")
{
  uv_loop_t* loop = uv_default_loop();

  // setup client
  uv_tcp_t* socket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, socket);
  //uv_tcp_keepalive(socket, 1, 6000);

  zmtp_stream_t* zmtp_stream = zmtp_stream_new((uv_stream_t*)socket, client_on_read);
  zmtp_stream_connect_t* connect_req = (zmtp_stream_connect_t*)malloc(sizeof(zmtp_stream_connect_t));
  int res = zmtp_stream_connect(connect_req, zmtp_stream, "<not yet parsed address>", client_on_connect);

  // setup server
  uv_tcp_t* server = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, server);


  zmtp_stream_t* s = zmtp_stream_new((uv_stream_t*)server, server_echo_on_read);
  zmtp_stream_bind(s, "<not yet parsed address>");


  uv_run(loop, UV_RUN_DEFAULT);

}