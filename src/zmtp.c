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
  result->endpointstream = NULL;
  result->greetings = zmtp_greetings_new(3, 0, false, "NULL");
  return result;
}

// ---------------------------------------------
// destructor
// ---------------------------------------------
void zmtp_stream_delete(zmtp_stream_t* s)
{
  input_stream_delete(s->input_stream);
  zmtp_greetings_delete(s->greetings);
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
  printf("parsed peer version=%d\n", version);
  return version;
}

// ---------------------------------------------
// parse first part of greetings
// ---------------------------------------------
static int parse_greetings_2(input_stream_t* is)
{
  printf("parse whole greetings\n");
  char* data = input_stream_data(is);
  int minor_version = zmtp_parse_minor_version(data);
  bool as_server = zmtp_parse_as_server(data);
  char* mechanism = zmtp_parse_mechanism(data);
  printf("peer : minor version=%d, as server=%d, mechanism='%s'\n", minor_version,as_server,mechanism);
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
  if (nread == UV_EOF)
  {
    // TODO handle EOF
    printf("EOF\n");
    return;
  }
  if (nread < 0)
  {
    printf("on read error : '%s'\n", uv_strerror((int)nread));
    exit(1);
  }
  printf("on read %d bytes\n", (int)nread);
  zmtp_stream_t* zmtp_stream = client->data;
  input_stream_t* is = zmtp_stream->input_stream;
  input_stream_append(is, buf->base, nread);

  if (buf && buf->base)
    free(buf->base);

  if (zmtp_stream->status==greetings1 && input_stream_size(is) >= ZMTP_GREETINGS_START_LEN) // first greetings received
  {
    int version = parse_greetings_1(is);
    if (version != 3)
    {
      printf("Only ZMTP v3.0 is supported. Abort.\n");
      exit(1);
    }
    // now send main part of greetings
    input_stream_pop(is, ZMTP_GREETINGS_START_LEN);
    zmtp_stream->status = greetings2;
    char minor_version = 0x00;
    bool as_server = false;
    //printf("stream status read %d write %d\n", uv_is_readable(zmtp_stream->stream), uv_is_writable(zmtp_stream->stream));
    
    zmtp_send_greetings_end(zmtp_stream->greetings, zmtp_stream->stream);
  }
  //printf("stream status read %d write %d\n", uv_is_readable(zmtp_stream->stream), uv_is_writable(zmtp_stream->stream));
  if (zmtp_stream->status == greetings2 && input_stream_size(is) >= ZMTP_GREETINGS_END_LEN)
  {
    int res = parse_greetings_2(is);
    input_stream_pop(is, ZMTP_GREETINGS_END_LEN);
    zmtp_stream->status = frame;
    printf("ready for frames\n");
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
    exit(1);
  }
  zmtp_stream_t* stream = (zmtp_stream_t*)(req->data);
  req->handle->data = stream;
  // start reading
  int res = uv_read_start(req->handle, alloc_buffer, zmtp_on_read);
  if (res < 0)
  {
    printf("uv read start error : '%s'\n", uv_strerror(res));
    exit(1);
  }

  zmtp_send_greetings_start(stream->greetings, req->handle);
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

  int status =  uv_tcp_connect(connect, (uv_tcp_t*)stream->stream, (const struct sockaddr*)&dest, on_connect);
  if (status < 0)
  {
    printf("uv_tcp_connect error '%s'", uv_strerror(status));
    exit(1);
  }
  return 0;
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
  zmtp_stream_t* zmtp_stream = (zmtp_stream_t*)stream->data;

  zmtp_stream->stream = (uv_stream_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(stream->loop, (uv_tcp_t*)zmtp_stream->stream);
  zmtp_stream->stream->data = zmtp_stream; // pass zmtp_stream
  if (uv_accept(stream, (uv_stream_t*)zmtp_stream->stream) == 0)
  {
    uv_read_start((uv_stream_t*)zmtp_stream->stream, alloc_buffer, zmtp_on_read);
    zmtp_send_greetings_start(zmtp_stream->greetings,(uv_stream_t*)zmtp_stream->stream);
  }
  else {
    fprintf(stderr, "Uv_accept error \n");
  }
}

// ---------------------------------------------
// bind API function
// ---------------------------------------------
int zmtp_stream_bind(zmtp_stream_t* stream, const char* address)
{
  struct sockaddr_in addr;
  stream->endpointstream = stream->stream;
  uv_ip4_addr("0.0.0.0", 7000, &addr); // TODO parse address
  stream->endpointstream->data = stream;

  int res = uv_tcp_bind((uv_tcp_t*)stream->endpointstream, (const struct sockaddr*)&addr, 0);
  if (res < 0)
  {
    printf("uv_tcp_bind error '%s'", uv_strerror(res));
    exit(1);
  }
  int BACKLOG = 128;
  int r = uv_listen(stream->endpointstream, BACKLOG, on_new_connection);
  if (r) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    return 1;
  }
  else
    return 0;
}


