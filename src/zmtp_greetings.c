#include "zmtp_greetings.h"


static void zmtp_greetings_sent(uv_write_t* req, int status)
{
  if (status < 0)
  {
    printf("greetings send failed: '%s'\n", uv_strerror(status));
    exit(1);
  }
  uv_buf_t* buf = (uv_buf_t*)req->data;
  printf("%d bytes greetings sucessfully sent.\n", buf->len);
  free(buf->base);
  free(req);
}

// ---------------------------------------------
// send first part of greetings
// ---------------------------------------------
void zmtp_send_greetings_start(uv_stream_t* stream, char major_version)
{
  printf("send greetings part 1...");
  uv_write_t* write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
  write_req->handle = stream;
  uv_buf_t* buf = (uv_buf_t*)malloc(sizeof(uv_buf_t));
  write_req->data = buf;

  ULONG size = 11;
  buf->base = (char*)malloc(size);
  buf->len = size;
  buf->base[0] = 0xFF;
  for (int i = 1; i < 9; i++)
    buf->base[i] = 0x00;
  buf->base[9] = 0x7F;
  buf->base[10] = major_version;


  int status = uv_write(write_req, stream, buf, 1, zmtp_greetings_sent);
  if (status < 0)
    printf("uv write error : '%s'\n", uv_strerror(status));
  else
    printf("done.\n");
}

// ---------------------------------------------
// send second part of greetings
// ---------------------------------------------
void zmtp_send_greetings_end(uv_stream_t* stream, char minor_version, char* mechanism, int mechanism_len, bool as_server)
{
  printf("send greetings part 2...");
  uv_write_t* write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
  write_req->handle = stream;
  uv_buf_t* buf = (uv_buf_t*)malloc(sizeof(uv_buf_t));
  write_req->data = buf;

  ULONG size = 53;
  buf->base = (char*)malloc(size);
  buf->len = size;
  for (ULONG i = 0; i < size; i++)
    buf->base[i] = 0x00;

  buf->base[0] = minor_version;
  if (mechanism_len > 20) mechanism_len = 20;
  memcpy(buf->base+1, mechanism, mechanism_len);
  buf->base[21] = as_server ? 0x01 : 0x00;

  int status = uv_write(write_req, stream, buf, 1, zmtp_greetings_sent);
  if (status < 0) 
    printf("uv write error : '%s'\n", uv_strerror(status));
  else
    printf("done.\n");
}


// ---------------------------------------------
// parse first part of greetigns
// ---------------------------------------------
int zmtp_parse_greetings_1(char* data)
{
  if (data[0] != (char)0xFF || data[9] != (char)0x7F)
  {
    printf("ZMTP signature not recognized. abort.\n");
    exit(1);
  }
  int version = (int)data[10];
  printf("parsed peer version=%d\n", version);
  return version;
}

// ---------------------------------------------
// parse minor version from second greetings
// ---------------------------------------------
int zmtp_parse_minor_version(char* data)
{
  int minor_version = (int)data[0];
  printf("peer minor version=%d\n", minor_version);
  return minor_version;
}

// ---------------------------------------------
// parse "as_server" flag
// ---------------------------------------------
bool zmtp_parse_as_server(char* data)
{
  return data[21] == 0x01;
}

// ---------------------------------------------
// return pointer to the mechansim data
// ---------------------------------------------
char* zmtp_parse_mechanism(char* data)
{
  return data + 1; // TODO allocation ?
}
