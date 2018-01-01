#include "zmtp_greetings.h"
#include <stdlib.h>

// ---------------------------------------------
// constructor
// ---------------------------------------------
zmtp_greetings_t* zmtp_greetings_new(int major, int minor, bool as_server, char* mechansim)
{
  zmtp_greetings_t* result = (zmtp_greetings_t*)malloc(sizeof(zmtp_greetings_t));
  result->major_version = major;
  result->minor_version = minor;
  result->as_server = as_server;
  strcpy(result->mechanism,  mechansim);
  return result;
}

// ---------------------------------------------
// destructor
// ---------------------------------------------
void zmtp_greetings_delete(zmtp_greetings_t* g)
{
  free(g);
}

// ---------------------------------------------
// build greetings head
// ---------------------------------------------
char* zmtp_greetings_head(zmtp_greetings_t* g)
{
  const int size = 11;
  char *result = (char*)malloc(size * sizeof(char));
  for (int i = 0; i < size; i++)
    result[i] = 0x00;
  result[0] = 0xFF;
  result[9] = 0x7F;
  result[10] = g->major_version;
  return result;
}

// ---------------------------------------------
// build greetings tail
// ---------------------------------------------
char* zmtp_greetings_tail(zmtp_greetings_t* g)
{
  const int size = 53;
  char *result = (char*)malloc(size * sizeof(char));
  for (int i = 0; i < size; i++)
    result[i] = 0x00;
  result[0] = g->minor_version;
  strcpy(result + 1,  g->mechanism);
  result[21] = g->as_server ? 0x01 : 0x00;
  return result;
}

// ---------------------------------------------
// test if tow instances matches on version and 
// mechasnim
// ---------------------------------------------
bool zmtp_greetings_match(zmtp_greetings_t* a, zmtp_greetings_t* b)
{
  return (a->major_version == b->major_version)
    && (a->minor_version == b->minor_version)
    && (strcmp(a->mechanism, b->mechanism) == 0);
}

// ---------------------------------------------
// send callback
// ---------------------------------------------
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
  free(buf);
  free(req);
}


// ---------------------------------------------
// send first part of greetings
// ---------------------------------------------
void zmtp_send_greetings_start(zmtp_greetings_t* greetings, uv_stream_t* stream)
{
  printf("send greetings part 1...");
  uv_write_t* write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
  write_req->handle = stream;
  uv_buf_t* buf = (uv_buf_t*)malloc(sizeof(uv_buf_t));
  write_req->data = buf;

  char* content = zmtp_greetings_head(greetings);
  unsigned long size = 11;
  buf->base = content;
  buf->len = size;

  int status = uv_write(write_req, stream, buf, 1, zmtp_greetings_sent);
  if (status < 0)
    printf("uv write error : '%s'\n", uv_strerror(status));
  else
    printf("done.\n");
}

// ---------------------------------------------
// send second part of greetings
// ---------------------------------------------
void zmtp_send_greetings_end(zmtp_greetings_t* greetings, uv_stream_t* stream)
{
  printf("send greetings part 2...");
  uv_write_t* write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
  write_req->handle = stream;
  uv_buf_t* buf = (uv_buf_t*)malloc(sizeof(uv_buf_t));
  write_req->data = buf;

  char* content = zmtp_greetings_tail(greetings);
  unsigned long size = 53;
  buf->base = content;
  buf->len = size;

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
