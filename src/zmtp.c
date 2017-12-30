#include "zmtp.h"

// ---------------------------------------------
// helper function for network order handling
// ---------------------------------------------
uint64_t nw_order(const uint64_t in) {
  unsigned char out[8] = { in >> 56,in >> 48,in >> 40,in >> 32,in >> 24,in >> 16,in >> 8,in };
  return *(uint64_t*)out;
}

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
  result->connect_cb = NULL;
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
static int parse_greetings_2(zmtp_greetings_t* self_greetings, input_stream_t* is)
{
  printf("parse whole greetings\n");
  char* data = input_stream_data(is);
  int minor_version = zmtp_parse_minor_version(data);
  bool as_server = zmtp_parse_as_server(data);
  char* mechanism = zmtp_parse_mechanism(data);
  printf("peer : minor version=%d, as server=%d, mechanism='%s'\n", minor_version,as_server,mechanism);
  if (strcmp(mechanism, self_greetings->mechanism) != 0)
  {
    printf("incompatible mechanims. abort.");
    exit(1);
  }
  if (minor_version != self_greetings->minor_version)
  {
    printf("incompatible minor version. abort.");
    exit(1);
  }
  // TODO store as_server info
  return 0;
}

static void on_msg_sent(uv_write_t* req, int status)
{
  if (status < 0)
  {
    printf("msg send failed: '%s'\n", uv_strerror(status));
    exit(1);
  }
  uv_buf_t* buf = (uv_buf_t*)req->data;
  printf("%d bytes sucessfully sent.\n", buf->len);
  free(buf->base);
}

// ---------------------------------------------
// send data
// ---------------------------------------------
int zmtp_stream_send(zmtp_stream_t* stream, void* data, size_t size)
{
  uv_stream_t* s = stream->stream;

  bool long_message = size > 255;
  int header_size = long_message ? 9 : 2;
  char *header = (char*)malloc(header_size * sizeof(char));
  if (long_message)
  {
    header[0] = 0x02;
    uint64_t l = nw_order(size);
    memcpy(header + 1, &l, 8);
  }
  else
  {
    header[0] = 0x00;
    header[1] = (unsigned char)size;
  }
  // send header
  uv_write_t* header_write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
  header_write_req->handle = s;
  uv_buf_t* header_buf = (uv_buf_t*)malloc(sizeof(uv_buf_t));
  header_write_req->data = header_buf;

  header_buf->base = header;
  header_buf->len = header_size;
  int status = uv_write(header_write_req, s, header_buf, 1, on_msg_sent);
  if (status < 0)
    printf("uv write error : '%s'\n", uv_strerror(status));

  // send content
  uv_write_t* content_write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
  content_write_req->handle = s;
  uv_buf_t* content_buf = (uv_buf_t*)malloc(sizeof(uv_buf_t));
  content_write_req->data = content_buf;
  content_buf->base = data;
  content_buf->len = (ULONG)size;
  status = uv_write(content_write_req, s, content_buf, 1, on_msg_sent);
  if (status < 0)
    printf("uv write error : '%s'\n", uv_strerror(status));
  return (int)size;
}

// ---------------------------------------------
// parse data frame
// ---------------------------------------------
static void parse_frame(zmtp_stream_t* zmtp_stream)
{
  // parse and check frame size, and if everything is ok call user read_cb

  input_stream_t* is = zmtp_stream->input_stream;
  size_t size = input_stream_size(is);
  if( size<1 ) return;
  char* data = input_stream_data(is);
  char first_byte = data[0];

  if (first_byte == 0x00 || first_byte == 0x02) // message last
  {
    int header_size = 1 + ((first_byte == 0x00) ? 1 : 8); // short or long messasge
    if (input_stream_size(is) < header_size) return;
    size_t frame_size = (int)data[1];
    if (first_byte == 0x02)
      frame_size = nw_order(atoll(data + 1));
    if (size < frame_size + header_size)
      return; // whole frame not yet received
    else
    {
      char* frame_content = data + header_size;
      if (zmtp_stream->read_cb)
        zmtp_stream->read_cb(zmtp_stream, frame_content, frame_size);
      input_stream_pop(is,header_size + frame_size);
    }
  }
  else if (first_byte == 0x01 || first_byte== 0x03) // short message more
  {
    printf("multi frame messages not supported yet");
    exit(1);
  }
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
  zmtp_stream_connect_t* z_req = client->data;
  zmtp_stream_t* zmtp_stream = z_req->stream;
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
    
    zmtp_send_greetings_end(zmtp_stream->greetings, zmtp_stream->stream);
  }
  if (zmtp_stream->status == greetings2 && input_stream_size(is) >= ZMTP_GREETINGS_END_LEN)
  {
    int res = parse_greetings_2(zmtp_stream->greetings, is);
    input_stream_pop(is, ZMTP_GREETINGS_END_LEN);
    zmtp_stream->status = frame;
    printf("ready for frames\n");
    // TODO security handshake
    if (zmtp_stream->connect_cb)
      zmtp_stream->connect_cb(z_req, 0);
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
  zmtp_stream_connect_t* z_req = (zmtp_stream_connect_t*)(req->data);
  if (status < 0)
  {
    if (z_req->stream->connect_cb)
      z_req->stream->connect_cb(z_req, status);
    else
    {
      printf("on connect error ");
      printf("%s\n", uv_strerror(status));
      exit(1);
    }
  }
  zmtp_stream_t* z_stream = z_req->stream;
  req->handle->data = z_req;
  // start reading
  int res = uv_read_start(req->handle, alloc_buffer, zmtp_on_read);
  if (res < 0)
  {
    printf("uv read start error : '%s'\n", uv_strerror(res));
    exit(1);
  }

  zmtp_send_greetings_start(z_stream->greetings, req->handle);
}

// ---------------------------------------------
// connect API function
// ---------------------------------------------
int zmtp_stream_connect(zmtp_stream_connect_t* z_req, zmtp_stream_t* z_stream, const char* address, zmtp_stream_connect_cb connect_cb)
{
  z_req->stream = z_stream;
  uv_connect_t* connect_req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
  connect_req->data = z_req;
  z_stream->connect_cb = connect_cb;

  // if TCP
  struct sockaddr_in dest; // TODO parse address, handle pipe_t
  uv_ip4_addr("127.0.0.1", 7000, &dest);

  int status =  uv_tcp_connect(connect_req, (uv_tcp_t*)z_stream->stream, (const struct sockaddr*)&dest, on_connect);
  if (status < 0)
  {
    printf("uv_tcp_connect error '%s'", uv_strerror(status));
    exit(1);
  }
  return status;
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
  zmtp_stream_connect_t* z_req = (zmtp_stream_connect_t*)malloc(sizeof(zmtp_stream_connect_t));
  z_req->stream = zmtp_stream;
  zmtp_stream->stream->data = z_req;
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


