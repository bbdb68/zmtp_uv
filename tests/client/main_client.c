#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "uv.h"
#include "zmtp.h"
#include "input_stream.h"


// -------------------------
// main client
// -------------------------
int main()
{
  printf("Hello, I am the client\n");
  uv_loop_t* loop = uv_default_loop();
  
  uv_tcp_t* socket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, socket);

  zmtp_stream_t* zmtp_stream = zmtp_stream_new((uv_stream_t*)socket, NULL);
  int res = zmtp_stream_connect(zmtp_stream, "<not yet parsed address>");

  uv_run(loop, UV_RUN_DEFAULT);

  _CrtDumpMemoryLeaks();
  return 0;
}