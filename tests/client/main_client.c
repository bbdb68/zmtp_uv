#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "uv.h"

void zmtp_send_greetings(uv_stream_t* stream)
{
  // send start of ZMTP greetings
  uv_write_t* write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
  write_req->handle = stream;
  uv_buf_t* buf = (uv_buf_t*)malloc(sizeof(uv_buf_t));

  ULONG size = 11;
  buf->base = (char*)malloc(size);
  buf->len = size;
  buf->base[0] = 0xFF;
  for (int i = 1; i < 9; i++)
    buf->base[i] = 0x00;
  buf->base[9] = 0x7F;
  buf->base[10] = 0x03;

  int res = uv_write(write_req, stream, buf, 1, NULL); // synchonous write
  if (res < 0)
  {
    printf("uv_write error : '%s'\n", uv_strerror(res));
    return;
  }
  else
    printf("11 byte greetings send\n");
  free(buf->base);
  free(buf);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = (ULONG)suggested_size;
}


void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
  printf("client on read %i\n", (int)nread);
  if (nread > 0) {
    printf("response = %s\n", buf->base);
  }
  //uv_close((uv_handle_t*)client, NULL);
}




void on_connect(uv_connect_t* req, int status)
{
  if (status < 0)
  {
    printf("on connect error ");
    printf("%s\n", uv_strerror(status));
    return;
  }
  // start reading
  int res = uv_read_start(req->handle, alloc_buffer, on_read);
  if (res < 0)
  {
    printf("uv read start error : '%s'\n", uv_strerror(res));
    return;
  }

  zmtp_send_greetings(req->handle);
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

  uv_connect_t* connect = (uv_connect_t*)malloc(sizeof(uv_connect_t));

  struct sockaddr_in dest;
  uv_ip4_addr("127.0.0.1", 7000, &dest);

  uv_tcp_connect(connect, socket, (const struct sockaddr*)&dest, on_connect);

  uv_run(loop, UV_RUN_DEFAULT);

  _CrtDumpMemoryLeaks();
  return 0;
}