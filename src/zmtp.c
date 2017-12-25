#include "zmtp.h"

// ---------------------------------------------
// constructor
// ---------------------------------------------
zmtp_stream_t* zmtp_stream_new(uv_stream_t* stream, zmtp_stream_read_cb read_cb)
{
  zmtp_stream_t* result = (zmtp_stream_t*)malloc(sizeof(zmtp_stream_t));
  result->input_stream = input_stream_new(1024);
  result->status = greetings1;
  result->stream = stream;
  result->read_cb = read_cb;
  return result;
}

// ---------------------------------------------
// destructor
// ---------------------------------------------
void zmtp_stream_delete(zmtp_stream_t* s)
{
  input_stream_delete(s->input_stream);
  free(s);
}

// ---------------------------------------------
// parse first part of greetigns
// ---------------------------------------------
static int parse_greetings_1(input_stream_t* is)
{
  char* data = input_stream_data(is);
  if (data[0] != (char)0xFF || data[9] != (char)0x7F)
  {
    printf("ZMTP signature not recognized. abort.\n");
    exit(1);
  }
  int version = (int)data[10];
  printf("peer version=%d\n", version);
  if (version != 3)
  {
    printf("Only ZMTP version 3 is supported. abort.");
    exit(1);
  }
  input_stream_pop(is, 11);
  return version;
}

// ---------------------------------------------
// parse first part of greetigns
// ---------------------------------------------
static int parse_greetings_2(input_stream_t* is)
{
  return 0;
}

// ---------------------------------------------
// parse data frame
// ---------------------------------------------
static void parse_frame(zmtp_stream_t* zmtp_stream)
{
  // parse and check frame size, and if everything is ok call user read_cb
}

// ---------------------------------------------
// on read
// ---------------------------------------------
static void zmtp_on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
  zmtp_stream_t* zmtp_stream = client->data;
  input_stream_t* is = zmtp_stream->input_stream;
  input_stream_append(is, buf->base, nread);

  if (zmtp_stream->status==greetings1 && input_stream_size(is) >= 11) // first greetings received
  {
    int version = parse_greetings_1(is);
    zmtp_stream->status = greetings2;
    // ok, so send main part of greetings

  }
  if (zmtp_stream->status == greetings2 && input_stream_size(is) >= 31)
  {
    int res = parse_greetings_2(is);
    zmtp_stream->status = frame;
  }
  if (zmtp_stream->status == frame)
  {
    parse_frame(zmtp_stream);
  }
}

// ---------------------------------------------
// alloc buffer
// ---------------------------------------------
static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = (ULONG)suggested_size;
}

// ---------------------------------------------
// on connect
// ---------------------------------------------
static void on_connect(uv_connect_t* req, int status)
{
  if (status < 0)
  {
    printf("on connect error ");
    printf("%s\n", uv_strerror(status));
    return;
  }
  zmtp_stream_t* stream = (zmtp_stream_t*)(req->data);
  req->handle->data = stream;
  // start reading
  int res = uv_read_start(req->handle, alloc_buffer, zmtp_on_read);
  if (res < 0)
  {
    printf("uv read start error : '%s'\n", uv_strerror(res));
    return;
  }

  zmtp_send_greetings(req->handle);
}

// ---------------------------------------------
// connect API function
// ---------------------------------------------
int zmtp_stream_connect(zmtp_stream_t* stream, const char* address)
{
  uv_connect_t* connect = (uv_connect_t*)malloc(sizeof(uv_connect_t));
  connect->data = stream;

  // if TCP
  struct sockaddr_in dest; // TODO parse address, handle pipe_t
  uv_ip4_addr("127.0.0.1", 7000, &dest);

  return uv_tcp_connect(connect, (uv_tcp_t*)stream->stream, (const struct sockaddr*)&dest, on_connect);
}

// ---------------------------------------------
// handle new connection for bound socket
// ---------------------------------------------
static void on_new_connection(uv_stream_t *stream, int status)
{
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    // error!
    return;
  }

  uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(stream->loop, client);
  client->data = stream->data; // pass zmtp_stream
  if (uv_accept(stream, (uv_stream_t*)client) == 0)
  {
    uv_read_start((uv_stream_t*)client, alloc_buffer, zmtp_on_read);
    zmtp_send_greetings((uv_stream_t*)client);
  }
  else {
    fprintf(stderr, "New uv_accept error \n");
//    uv_close((uv_handle_t*)client, on_close);
  }
}

// ---------------------------------------------
// bind API function
// ---------------------------------------------
int zmtp_stream_bind(zmtp_stream_t* stream, const char* address)
{
  struct sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", 7000, &addr); // TODO parse address
  stream->stream->data = stream;

  int res = uv_tcp_bind((uv_tcp_t*)stream->stream, (const struct sockaddr*)&addr, 0);
  // TODO check res
  int BACKLOG = 128;
  int r = uv_listen(stream->stream, BACKLOG, on_new_connection);
  if (r) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    return 1;
  }
  else
    return 0;
}


// ---------------------------------------------
// send first part of greetings
// TODO make method of zmtp_stream
// ---------------------------------------------
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
  buf->base[10] = 0x03; // zmtp version 3

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
