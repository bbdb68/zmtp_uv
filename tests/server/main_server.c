#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define DEFAULT_PORT 7000
#define DEFAULT_BACKLOG 128

uv_loop_t *loop;
struct sockaddr_in addr;

typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
  write_req_t *wr = (write_req_t*)req;
  free(wr->buf.base);
  free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = (ULONG)suggested_size;
}

void on_close(uv_handle_t* handle) {
  free(handle);
}

void echo_write(uv_write_t *req, int status) {
  if (status) {
    fprintf(stderr, "Write error %s\n", uv_strerror(status));
  }
  free_write_req(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) 
{
  printf("server on read %i\n", (int)nread);
  if (nread > 0) {
    write_req_t *req = (write_req_t*)malloc(sizeof(write_req_t));
    req->buf = uv_buf_init(buf->base, (unsigned int)nread);
    uv_write((uv_write_t*)req, client, &req->buf, 1, echo_write);
    printf("echo done\n");
    return;
  }
  if (nread < 0) 
  {
    if (nread != UV_EOF)
      fprintf(stderr, "Read error %s\n", uv_err_name((int)nread));
    else
      printf("EOF");
    uv_close((uv_handle_t*)client, on_close);
  }

  free(buf->base);
}

void read_zmtp_greetings(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
  printf("server on read_zmtp_greetings %i\n", (int)nread);
  if (nread > 0) 
  {
    //write_req_t *req = (write_req_t*)malloc(sizeof(write_req_t));
    //req->buf = uv_buf_init(buf->base, (unsigned int)nread);
    //uv_write((uv_write_t*)req, client, &req->buf, 1, echo_write);
    //printf("echo done\n");

    // greetings is accepted, lets fall back on default cb
    uv_read_stop(client);
    uv_read_start(client, alloc_buffer, echo_read);
    return;
  }
  if (nread < 0)
  {
    if (nread != UV_EOF)
      fprintf(stderr, "Read error %s\n", uv_err_name((int)nread));
    else
      printf("EOF");
    uv_close((uv_handle_t*)client, on_close);
  }

  free(buf->base);
}



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

void on_new_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    // error!
    return;
  }

  uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);
  if (uv_accept(server, (uv_stream_t*)client) == 0) 
  {
    uv_read_start((uv_stream_t*)client, alloc_buffer, read_zmtp_greetings);
    zmtp_send_greetings((uv_stream_t*)client);
  }
  else {
    uv_close((uv_handle_t*)client, on_close);
  }
}

int main() {
  loop = uv_default_loop();

  uv_tcp_t server;
  uv_tcp_init(loop, &server);

  uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);

  uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
  int r = uv_listen((uv_stream_t*)&server, DEFAULT_BACKLOG, on_new_connection);
  if (r) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    return 1;
  }
  return uv_run(loop, UV_RUN_DEFAULT);
}
