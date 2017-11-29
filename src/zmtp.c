#include "zmtp.h"

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
