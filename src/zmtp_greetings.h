#pragma once

#include "uv.h"
#include "input_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

// helper functions for zmtp greetings

void zmtp_send_greetings(uv_stream_t* stream, char major_version);


#ifdef __cplusplus
}
#endif