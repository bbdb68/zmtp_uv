#pragma once

#include "uv.h"
#include "input_stream.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// helper functions for zmtp greetings
#define ZMTP_GREETINGS_START_LEN 11
#define ZMTP_GREETINGS_END_LEN 53

void zmtp_send_greetings_start(uv_stream_t* stream, char major_version);
void zmtp_send_greetings_end(uv_stream_t* stream, char minor_version, char* mechanism, int mechanism_len, bool as_server);

int zmtp_parse_greetings_1(char* data);
int zmtp_parse_minor_version(char* data);
bool zmtp_parse_as_server(char* data);
char* zmtp_parse_mechanism(char* data);

#ifdef __cplusplus
}
#endif