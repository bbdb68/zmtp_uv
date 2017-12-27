#pragma once

#include "uv.h"
#include "input_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

// helper functions for zmtp greetings
#define ZMTP_GREETINGS_START_LEN 11
#define ZMTP_GREETINGS_END_LEN 53

void zmtp_send_greetings_start(uv_stream_t* stream, char major_version);

void zmtp_send_greetings_end(uv_stream_t* stream, char minor_version, char* mecanism, int mecanism_len, int as_server);


#ifdef __cplusplus
}
#endif