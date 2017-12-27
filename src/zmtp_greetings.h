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


typedef struct zmtp_greetings_s zmtp_greetings_t;

zmtp_greetings_t* zmtp_greetings_new(int major, int minor, bool as_server, char* mechansim);
void zmtp_greetings_delete(zmtp_greetings_t* g);

char* zmtp_greetings_head(zmtp_greetings_t* g);
char* zmtp_greetings_tail(zmtp_greetings_t* g);
bool zmtp_greetings_match(zmtp_greetings_t* a, zmtp_greetings_t* b);

void zmtp_send_greetings_start(zmtp_greetings_t* greetings, uv_stream_t* stream);
void zmtp_send_greetings_end(zmtp_greetings_t* greetings, uv_stream_t* stream);

int zmtp_parse_greetings_1(char* data);
int zmtp_parse_minor_version(char* data);
bool zmtp_parse_as_server(char* data);
char* zmtp_parse_mechanism(char* data);

// -------------------------------
// zmtp greetings definition
// -------------------------------
struct zmtp_greetings_s
{
  int major_version;
  int minor_version;
  char mechanism[20];
  bool as_server;
};




#ifdef __cplusplus
}
#endif